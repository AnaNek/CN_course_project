#include "ktracer.h"

MODULE_AUTHOR("Neklepaeva A.N.");
MODULE_LICENSE("GPL");

spinlock_t pid_locks[PID_MAX + 1];
struct proc_info **info_by_pid;

/* т-ка входа при открытии специального файла уст-ва */
static int tr_open(struct inode *in, struct file *filp)
{
    printk(LOG_LEVEL "TR device opened\n");
    return 0;
}

/* т-ка входа при закрытии специального файла уст-ва */
static int tr_release(struct inode *in, struct file *filp)
{
    printk(LOG_LEVEL "TR device closed \n");
    return 0;
}

/* Добавление pid к множеству отслеживаемых процессов */
static int add_process(int pid) 
{
    struct proc_info *p_info;
    
    if (pid > PID_MAX || pid < 0)
        return -EINVAL;
      
    spin_lock(&pid_locks[pid]);
      
    if (info_by_pid[pid] != NULL)
    {
        spin_unlock(&pid_locks[pid]);
        return 0;
    }

    p_info = kmalloc(sizeof(struct proc_info), GFP_KERNEL);
    if (p_info == NULL)
    {
        spin_unlock(&pid_locks[pid]);
        return -ENOMEM;
    }
            	
    p_info->index = 0;
    info_by_pid[pid] = p_info;
	
    spin_unlock(&pid_locks[pid]);

    return 0;
}

/* Удаление pid из множества отслеживаемых процессов */
static int remove_process(int pid) 
{
    if (pid > PID_MAX || pid < 0)
        return -EINVAL;
        
    spin_lock(&pid_locks[pid]);    
    if (info_by_pid[pid] != NULL)
    {
        kfree(info_by_pid[pid]);
        info_by_pid[pid] = NULL;
        spin_unlock(&pid_locks[pid]);
        return 0;
    }
        
    spin_unlock(&pid_locks[pid]);

    return -EINVAL;
}

/* получение информации о pid */
static int get_proc_info(void __user *argp)
{
    char buf[64];
    int len = 0;
    int total_len = 0;
    request_t req;
    int err = 0;
    int i = 0;

    /* копирование данных из пространства пользователя  пр-во ядра */
    if (copy_from_user(&req, (request_t __user *)argp, sizeof(req)))
    {
        printk("copy_from_user() failed");
        return -EFAULT;
    }
    
    if (req.pid > PID_MAX || req.pid < 0)
        return -EINVAL;
        		
    spin_lock(&pid_locks[req.pid]);
    if (info_by_pid[req.pid] != NULL)
    {   
        if (info_by_pid[req.pid]->index)
        {
            len = sprintf(buf, "to/from jiffies\n");
        }
        else
        {
            /* если о процессе нет информации, 
            записать в буфер соответсвующее сообщение */
            len = sprintf(buf, "No info about process with pid %d\n", req.pid);
        }
        total_len += len;
    
        if (total_len < req.len - 1)
        {
            strcpy(req.buf, buf);
        }         
        /* записать в буфер информацию о процессе */
        for (i = 0; i < info_by_pid[req.pid]->index && total_len < (req.len - 1); i++)
        {
            len = sprintf(buf, "%-6c%-9lld\n",
			info_by_pid[req.pid]->stats[i],
			atomic64_read(&info_by_pid[req.pid]->times[i]));
            total_len += len;
            
            if (total_len < req.len - 1)
            {
                strncat(req.buf, buf, len);
            }
        }
    }
    else
    {
        /* если процесс не отслеживается, записать 
        в буфер соответсвующее сообщение */
        total_len = sprintf(buf, "Process with pid %d is not monitored\n", req.pid);
    
        if (total_len < req.len - 1)
        {
            strcpy(req.buf, buf);
        } 
    }
    spin_unlock(&pid_locks[req.pid]);
    
    req.buf[total_len] = '\0';
    
    /* копирование данных из пространства ядра в пр-во пользователя */
    if (copy_to_user(argp, &req, sizeof(req)))
    {
        printk("copy_to_user() failed");
        err = -EFAULT;
    }
    			
    return 0;
}

/* Обработчик ioctl */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int tr_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long tr_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
    int ret = 0;
    void __user *argp = (void __user *)arg;
	
    switch (cmd) {
    case TRACER_ADD_PROCESS:
        printk(LOG_LEVEL "start monitoring process\n");
        ret = add_process(arg);
        break;
    case TRACER_REMOVE_PROCESS:
        printk(LOG_LEVEL "stop monitoring process\n");
        ret = remove_process(arg);
        break;
    case TRACER_GET_INFO:
        printk(LOG_LEVEL "request for process info\n");
        ret = get_proc_info(argp);
        break;
    default:
        ret = -ENOTTY;
    }
    return ret;
}

/* 
   определение файловых операций 
   над специальным файлом устройства
*/
const struct file_operations tr_fops = {
    .owner = THIS_MODULE,
    .open = tr_open,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = tr_ioctl,
#else
    .unlocked_ioctl = tr_ioctl,
#endif
    .release = tr_release
};

/* структура, описывающая устройство */
static struct miscdevice tracer_dev = {
    .minor  = TRACER_DEV_MINOR,
    .name   = TRACER_DEV_NAME,
    .fops   = &tr_fops,
};

static int ktracer_init(void)
{
    int ret = 0, i;

    /* инициализация спин-локов */
    for (i = 0; i < PID_MAX; i++)
        spin_lock_init(&pid_locks[i]);


    /* выделение памяти под ассоциативный массив */
    info_by_pid = kmalloc(sizeof(struct proc_info*) * (PID_MAX + 1), GFP_KERNEL);
    
    if (info_by_pid == NULL)
        return -ENOMEM;
    
    for (i = 0; i < PID_MAX + 1; i++)
    {
        info_by_pid[i] = NULL;
        printk("ptr %p\n",  info_by_pid[i]);
    }

    /* Регистрация хуков */
    ret = fh_init();
    if (ret) 
    {
        printk(LOG_LEVEL "Unable to register hooks\n");
        kfree(info_by_pid);
        return ret;
    }

	/* Регистрация устройства */
    if (misc_register(&tracer_dev)) 
    {
        printk(LOG_LEVEL "Unable to register device\n");
        fh_exit();
        kfree(info_by_pid);
        return -EINVAL;
    }

    printk(LOG_LEVEL "Device 'tracer' initiated\n");
	
    return 0;
}

static void ktracer_exit(void)
{
    int i = 0;
    
    /* Дерегистрация хуков */
    fh_exit();
    
    for (i = 0; i < PID_MAX + 1; i++)
    {
        if (info_by_pid[i] != NULL)
        {
            kfree(info_by_pid[i]);
        }
    }
    
    /* освобождение памяти, выделенной для
       ассоциативного массива
    */
    kfree(info_by_pid);
    
    /* Дерегистрация устройства */
    misc_deregister(&tracer_dev);
    
    printk(LOG_LEVEL "Device 'tracer' removed\n");
}

module_init(ktracer_init);
module_exit(ktracer_exit);
