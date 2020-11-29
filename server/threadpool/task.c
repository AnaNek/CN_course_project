#include <stdlib.h>

#include "task.h"
#include "../types.h"

struct task {
    work_t work;
    void* arg;
    struct task* next;
};

struct task_array {
    int size;
    task_t** tasks;
};

int task_init(task_t** holder, work_t work, void* arg, task_t* next) {
    if (holder == NULL || work == NULL) {
        return ERR_PARAMS;
    }

    task_t* task = (task_t *) malloc(sizeof(task_t));
    if (task == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    task->work = work;
    task->arg = arg;
    task->next = next;
    *holder = task;

    return SUCCESS;
}

int task_set_next(task_t* task, task_t* next) {
    if (task == NULL) {
        return ERR_PARAMS;
    }
    task->next = next;
    return SUCCESS;
}

task_t* task_get_next(task_t* task) {
    if (task == NULL) {
        return NULL;
    }
    return task->next;
}

int task_array_create(task_array_t** holder, int size) {
    if (holder == NULL || size <= 0) {
        return ERR_PARAMS;
    }

    task_array_t* task_array = (task_array_t*) malloc(sizeof(task_array_t));
    if (task_array == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    task_array->tasks = (task_t**) malloc(size * sizeof(task_t*));
    if (task_array->tasks == NULL) {
        free(task_array);
        return ERR_MEMORY_ALLOCATION;
    }

    for (int i = 0; i < size; i++) {
        task_array->tasks[i] = NULL;
    }

    task_array->size = size;
    *holder = task_array;
    return SUCCESS;
}

void task_array_free(task_array_t* holder) {
    if (holder == NULL) {
        return;
    }
    free(holder->tasks);
    free(holder);
}

task_t* get_task(task_array_t* array, int index) {
    if (array == NULL || index < 0 || index >= array->size || array->tasks == NULL) {
        return NULL;
    }
    return array->tasks[index];
}

int add_task(task_array_t* array, int index, task_t* task) {
    if (array == NULL || index < 0 || index >= array->size || array->tasks == NULL) {
        return ERR_PARAMS;
    }
    array->tasks[index] = task;
    return SUCCESS;
}

void task_perform(task_t* task) {
    if (task == NULL) {
        return;
    }
    (task->work)(task->arg);
}

void task_free(task_t* task) {
    free(task);
}
