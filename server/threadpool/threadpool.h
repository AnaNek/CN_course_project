#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "task.h"

typedef struct threadpool threadpool_t;

int threadpool_init(threadpool_t** holder, int threads_limit);

void threadpool_free(threadpool_t* threadpool);

int threadpool_add_work(threadpool_t* threadpool, work_t work, void* arg);


#endif
