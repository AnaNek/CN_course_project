#ifndef TRACER_H__
#define TRACER_H__ 1

#include <asm/ioctl.h>
#ifndef __KERNEL__
#include <sys/types.h>
#endif /* __KERNEL__ */

#define TRACER_DEV_MINOR 42
#define TRACER_DEV_NAME "tracer"

#define DRIVER_BUF_SIZE 500

typedef struct driver_request {
    int pid;
    int len;
    char buf[DRIVER_BUF_SIZE];
} driver_request_t;

#define TRACER_ADD_PROCESS     _IOW(_IOC_WRITE, 42, pid_t)
#define TRACER_REMOVE_PROCESS  _IOW(_IOC_WRITE, 43, pid_t)
#define TRACER_GET_INFO        _IOWR(_IOC_WRITE|_IOC_READ, 44, driver_request_t *)

#endif /* TRACER_H_ */
