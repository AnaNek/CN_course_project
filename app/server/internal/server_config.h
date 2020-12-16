#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

typedef struct config server_config_t;

int server_config_init(server_config_t** holder,
    int port, int threads_limit);

void server_config_free(server_config_t* config);

int get_port(server_config_t* config);

#endif
