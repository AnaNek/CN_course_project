#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"
#include "../types.h"

void test_threadpool_init_destroy_0_threads() {
    threadpool_t* pool = NULL;
    int got = threadpool_init(&pool, 0);
    int expect = ERR_PARAMS;
    printf("Expect rc to equal %d got %d\n", got, expect);
    assert(got == expect);
}

void test_threadpool_init_destroy_neg_threads() {
    threadpool_t* pool = NULL;
    int got = threadpool_init(&pool, -2);
    int expect = ERR_PARAMS;
    printf("Expect rc to equal %d got %d\n", got, expect);
    assert(got == expect);
}

void test_threadpool_init_destroy_nil_pool() {
    int got = threadpool_init(NULL, 4);
    int expect = ERR_PARAMS;
    printf("Expect rc to equal %d got %d\n", got, expect);
    assert(got == expect);
}

void test_threadpool_init_destroy_regular() {
    threadpool_t* pool = NULL;
    int got = threadpool_init(&pool, 16);
    int expect = SUCCESS;
    printf("Expect rc to equal %d got %d\n", got, expect);
    assert(got == expect);

    threadpool_free(pool);
}



int main(int argc, char const *argv[]) {
    test_threadpool_init_destroy_0_threads();

    test_threadpool_init_destroy_neg_threads();

    test_threadpool_init_destroy_nil_pool();

    test_threadpool_init_destroy_regular();

    return 0;
}
