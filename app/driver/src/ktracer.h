#ifndef __KTRACER_H__
#define __KTRACER_H__		1

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/cdev.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/ftrace.h>
#include <linux/tracepoint.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/percpu.h>
#include <trace/events/sched.h>
#include <linux/sched/clock.h>

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include "tracer.h"

#define TIME_BUF 28

#define BUFFER_SIZE		256

#define LOG_LEVEL		KERN_ALERT

#define TRACER_MAJOR		10
#define NUM_MINORS		1

#define PID_MAX                32768

#ifndef BUFSIZ
#define BUFSIZ			4096
#endif

extern spinlock_t pid_locks[PID_MAX + 1];

struct proc_info {
	int index;
	atomic64_t times[TIME_BUF];
	char stats[TIME_BUF];
};

extern struct proc_info **info_by_pid;

int fh_init(void);
void fh_exit(void);

#endif
