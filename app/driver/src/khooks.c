#include "ktracer.h"

#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/types.h>

#define HOOK(_name, _function, _original)   \
    {                                       \
        .name = (_name),                    \
        .fake_func = (_function),           \
        .orig_func = (_original),           \
    }

/* struct ftrace_hook - описание хука*/
struct ftrace_hook {
    const char *name;
    void *fake_func;
    void *orig_func;

    unsigned long address;
    struct ftrace_ops ops;
};

#define USE_FENTRY_OFFSET 0
#define pr_fmt(fmt) "ftrace_hook: " fmt

/* Получение адреса перехватываемой функции */
static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
    if (!(hook->address = kallsyms_lookup_name(hook->name))) 
    {
        pr_debug("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }

#if USE_FENTRY_OFFSET
    *((unsigned long*) hook->orig_func) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long*) hook->orig_func) = hook->address;
#endif

    return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
        struct ftrace_ops *ops, struct pt_regs *regs)
{
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    regs->ip = (unsigned long) hook->fake_func;
#else
    if (!within_module(parent_ip, THIS_MODULE)) 
    {
        regs->ip = (unsigned long) hook->fake_func;
    }
#endif
}

/* fh_install_hook() - регистрация и активация хука */
int fh_install_hook(struct ftrace_hook *hook)
{
    int err;
    
    if ((err = fh_resolve_hook_address(hook))) 
    {
        return err;
    }

    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                      | FTRACE_OPS_FL_RECURSION_SAFE
                      | FTRACE_OPS_FL_IPMODIFY;

    if ((err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0))) 
    {
        pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }
    if ((err = register_ftrace_function(&hook->ops))) 
    {
        pr_debug("register_ftrace_fake_func() failed: %d\n", err);
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        return err;
    }

    return 0;
}

/* fh_remove_hook() - деактивация и дерегистрация хука */
void fh_remove_hook(struct ftrace_hook *hook)
{
    int err;

    if ((err = unregister_ftrace_function(&hook->ops))) 
    {
        pr_debug("unregister_ftrace_fake_func() failed: %d\n", err);
    }

    if ((err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0))) 
    {
        pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
    }
}

/* fh_install_hooks() - регистрация и активация нескольких хуков */
int fh_install_hooks(struct ftrace_hook *hooks, size_t count)
{
    int err;
    size_t i;

    for (i = 0; i < count; i++) 
    {
        err = fh_install_hook(&hooks[i]);

        if (err) 
        {
            while (i != 0) 
            {
                fh_remove_hook(&hooks[--i]);
            }
            break;
        }
    }
    
    return err;
}

/* fh_remove_hooks() - деактивация и дерегистрация нескольких хуков */
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++) 
    {
        fh_remove_hook(&hooks[i]);
    }
}

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

/* запись данных о процессе в буфер */
static void write_info(int pid, char state)
{
    if (pid <= PID_MAX)
    {
        spin_lock(&pid_locks[pid]);
        
        if (info_by_pid[pid] != NULL)
        {
            if (info_by_pid[pid]->index >= TIME_BUF)
            {
                info_by_pid[pid]->index = 0;
            }
            atomic64_set(&info_by_pid[pid]->times[info_by_pid[pid]->index], jiffies);
            info_by_pid[pid]->stats[info_by_pid[pid]->index] = state;
            info_by_pid[pid]->index += 1;
        }
        
        spin_unlock(&pid_locks[pid]);
    }
}

static struct rq * (*real_finish_task_switch)(struct task_struct *prev);

static struct rq * fh_finish_task_switch(struct task_struct *prev)
{
    write_info(prev->pid, '-');
    write_info(current->pid, '+');
    return real_finish_task_switch(prev);
}


static struct ftrace_hook tr_hooks[] = {
    HOOK("finish_task_switch",  fh_finish_task_switch,  &real_finish_task_switch),
};
int fh_init(void)
{
    return fh_install_hooks(tr_hooks, ARRAY_SIZE(tr_hooks));
}

void fh_exit(void)
{
    fh_remove_hooks(tr_hooks, ARRAY_SIZE(tr_hooks));
}
