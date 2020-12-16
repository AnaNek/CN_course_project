#include <stdio.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "threadpool/threadpool.h"
#include "internal/server.h"
#include "internal/handler.h"
#include "types.h"

volatile sig_atomic_t flag = 0;

void handle_interrupt(int sig_num) {
    printf("Interrupt: %d\n", sig_num);
    flag = 1;
}

void exit_on_fail(int rc) {
    if (rc != SUCCESS) {
        printf("Exiting with status %d\n", rc);
        printf("Errno state: %s\n", strerror(errno));
        exit(rc);
    }
}

void print_usage() {
    printf("Usage:\n ./server.out <port> <threads>\n");
    printf("Threads limit = %d\n", THREAD_MAX);
}

int are_valid(int port, int threads) {
    if (port < 1024 || port > 65535) {
        return FALSE;
    }
    if (threads <= 0 || threads > THREAD_MAX) {
        return FALSE;
    }

    return TRUE;
}

int main(int argc, char const *argv[]) {
    threadpool_t* pool = NULL;
    server_config_t* config = NULL;
    server_t* server = NULL;
    int client_sockfd = -1;
    int rc = SUCCESS;

    if (argc != ARGC) {
        print_usage();
        return rc;
    }

    int port = atoi(argv[1]);
    int threads = atoi(argv[2]);
    if (are_valid(port, threads) != TRUE) {
        print_usage();
        return rc;
    }

    signal(SIGINT, handle_interrupt);
    printf("Ctrl+C to exit established\n");

    rc = threadpool_init(&pool, threads);
    printf("Threadpool created with status: %d\n", rc);
    exit_on_fail(rc);

    rc = server_config_init(&config, port, threads);
    printf("Config init status: %d\n", rc);
    exit_on_fail(rc);

    rc = server_init(&server, config);
    printf("Server init status: %d\n", rc);
    exit_on_fail(rc);

    rc = server_run(server);
    printf("Server run status: %d\n", rc);
    exit_on_fail(rc);

    while (!flag) {
        rc = server_select(server);

        if (rc != SUCCESS) {
            if (rc == ERR_SELECT) {
                if (errno == EINTR) {
                    break;
                }
            }
            exit_on_fail(rc);
        }
        rc = server_accept(server, &client_sockfd);
        printf("Accepted new connection with status: %d, on sockfd: %d\n", rc, client_sockfd);

        if (rc != SUCCESS) {
            if (errno == EWOULDBLOCK) {
                continue;
            } else {
                exit_on_fail(rc);
            }
        } else {

            threadpool_add_work(pool, handle_new_connection, &client_sockfd);
        }
    }

    printf("Got interrupt signal. Gracefully stopping...\n");

    rc = server_stop(server);
    printf("Server stop status: %d\n", rc);
    printf("Errno state: %s\n", strerror(errno));
    threadpool_free(pool);
    server_config_free(config);
    server_free(server);

    return SUCCESS;
}
