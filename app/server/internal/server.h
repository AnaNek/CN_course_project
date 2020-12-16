#ifndef __SERVER_H__
#define __SERVER_H__

#include "server_config.h"

#define DEFAULT_POLL_SIZE 16

typedef struct server server_t;

int server_init(server_t** holder, server_config_t* config);

void server_free(server_t* server);

int is_running(server_t* s);

int set_socket(server_t* s, int sockfd);

int server_run(server_t* s);

int server_accept(server_t* s, int* accepted_sockfd);

int server_select(server_t* s);

int server_stop(server_t* s);

#endif
