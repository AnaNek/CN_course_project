#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "threadpool.h"
#include "../types.h"

void* perform_work(void* threadpool);

struct threadpool {
    int shutdown;
    int not_accept;
    int threads_limit;
    int task_count;
    task_t* thead;
    task_t* ttail;
    pthread_t* threads;
    pthread_cond_t cond_not_empty;
    pthread_cond_t cond_empty;
    pthread_mutex_t mutex;
};

task_t* get_current_task(threadpool_t* pool) {
    if (pool == NULL) {
        return NULL;
    }
    task_t* result = NULL;
    task_t* next = NULL;
    result = pool->thead;
    if (result == NULL) {
        return result;
    }

    next = task_get_next(result);
    if (next == NULL) {
        pool->thead = NULL;
        pool->ttail = NULL;
    } else {
        pool->thead = next;
    }
    return result;
}

int threadpool_init(threadpool_t** holder, int threads_limit) {
    if (holder == NULL || threads_limit <= 0) {
        return ERR_PARAMS;
    }

    int rc = 0;
    threadpool_t* threadpool = (threadpool_t*) malloc(sizeof(threadpool_t));
    if (threadpool == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    threadpool->shutdown = FALSE;
    threadpool->not_accept = FALSE;
    threadpool->threads_limit = threads_limit;
    threadpool->task_count = 0;
    threadpool->thead = NULL;
    threadpool->ttail = NULL;

    threadpool->threads = (pthread_t*) malloc(threads_limit * sizeof(pthread_t));
    if (threadpool->threads == NULL) {
        free(threadpool);
        return ERR_MEMORY_ALLOCATION;
    }

    rc = pthread_mutex_init(&(threadpool->mutex), NULL);
    if (rc != SUCCESS) {
        free(threadpool);
        return rc;
    }

    rc = pthread_cond_init(&(threadpool->cond_not_empty), NULL);
    if (rc != SUCCESS) {
        pthread_mutex_destroy(&(threadpool->mutex));
        free(threadpool);
        return rc;
    }

    rc = pthread_cond_init(&(threadpool->cond_empty), NULL);
    if (rc != SUCCESS) {
        pthread_mutex_destroy(&(threadpool->mutex));
        pthread_cond_destroy(&(threadpool->cond_not_empty));
        free(threadpool);
        return rc;
    }

    for (int i = 0; i < threads_limit; i++) {
        rc = pthread_create(&(threadpool->threads[i]), NULL, perform_work, threadpool);
        if (rc != SUCCESS) {
            free(threadpool->threads);
            free(threadpool);
            return ERR_CREATE_THREAD;
        }
        // pthread_detach(threadpool->threads[i]);
    }


    *holder = threadpool;
    return SUCCESS;
}

void threadpool_free(threadpool_t* threadpool) {
    if (threadpool == NULL) {
        return;
    }
    pthread_mutex_lock(&(threadpool->mutex));
    threadpool->not_accept = TRUE;

    while (threadpool->task_count != 0) {
        pthread_cond_wait(&(threadpool->cond_empty), &(threadpool->mutex));
    }

    threadpool->shutdown = TRUE;
    pthread_cond_broadcast(&(threadpool->cond_not_empty));
    pthread_mutex_unlock(&(threadpool->mutex));

    for (int i = 0; i < threadpool->threads_limit; i++) {
        pthread_join(threadpool->threads[i], NULL);
    }
    
    free(threadpool->threads);
    pthread_mutex_destroy(&(threadpool->mutex));
    pthread_cond_destroy(&(threadpool->cond_not_empty));
    pthread_cond_destroy(&(threadpool->cond_empty));
    free(threadpool);
    threadpool = NULL;
}

int threadpool_add_work(threadpool_t* threadpool, work_t work, void* arg) {
    if (threadpool == NULL) {
        return ERR_PARAMS;
    }

    task_t* task = NULL;
    int rc = task_init(&task, work, arg, NULL);
    if (rc != SUCCESS) {
        return rc;
    }

    pthread_mutex_lock(&(threadpool->mutex));

    if (threadpool->not_accept) {
        task_free(task);
        return ERR_ACCEPT;
    }


    if (threadpool->task_count == 0) {
        threadpool->thead = task;
        threadpool->ttail = task;
        pthread_cond_signal(&(threadpool->cond_not_empty));
    } else {
        task_set_next(threadpool->ttail, task);
        threadpool->ttail = task;
    }

    threadpool->task_count++;
    pthread_mutex_unlock(&(threadpool->mutex));
    return rc;
}

void* perform_work(void* threadpool) {
    if (threadpool == NULL) {
        return NULL;
    }
    threadpool_t* pool = (threadpool_t*) threadpool;
    task_t* current = NULL;

    while (TRUE) {
        pthread_mutex_lock(&(pool->mutex));
        while (pool->thead == NULL) {
            if (pool->shutdown) {
                pthread_mutex_unlock(&(pool->mutex));
                pthread_exit(NULL);
            }

            pthread_mutex_unlock(&(pool->mutex));
            pthread_cond_wait(&(pool->cond_not_empty), &(pool->mutex));

            if (pool->shutdown) {
                pthread_mutex_unlock(&(pool->mutex));
                pthread_exit(NULL);
            }
        }

        current = pool->thead;
        pool->task_count--;

        if (pool->task_count == 0) {
            pool->thead = NULL;
            pool->ttail = NULL;
        } else {
            pool->thead = task_get_next(current);
        }

        if (pool->task_count == 0 && (!pool->shutdown)) {
            pthread_cond_signal(&(pool->cond_empty));
        }
        pthread_mutex_unlock(&(pool->mutex));

        task_perform(current);
        task_free(current);
    }

    return NULL;
}
