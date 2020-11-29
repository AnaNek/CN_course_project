#ifndef __TASK_H__
#define __TASK_H__

typedef struct task task_t;

typedef struct task_array task_array_t;

typedef void (*work_t)(void*);

int task_init(task_t** task_holder, work_t work, void* arg, task_t* next);

int task_set_next(task_t* task, task_t* next);

task_t* task_get_next(task_t* task);

int task_array_create(task_array_t** holder, int size);

int add_task(task_array_t* array, int index, task_t* task);

task_t* get_task(task_array_t* array, int index);

void task_array_free(task_array_t* holder);

void task_free(task_t* task);

void task_perform(task_t* task);

#endif
