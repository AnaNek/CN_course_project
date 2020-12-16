#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "server_config.h"
#include "../types.h"

struct config {
    int port;
    int threads_limit;
};

// PRIVATE DECLARE
int is_port_valid(int port);

// PUBLIC DEFINE
int server_config_init(
    server_config_t** holder,
    int port,
    int threads_limit) {
    if (holder == NULL) {
        return ERR_PARAMS;
    }

    if (is_port_valid(port) != TRUE) {
        return ERR_INVALID_PORT;
    }

    server_config_t* config = (server_config_t*) malloc(sizeof(server_config_t));
    if (config == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    config->port = port;
    config->threads_limit = threads_limit;

    *holder = config;
    return SUCCESS;
}

void server_config_free(server_config_t* config) {
    free(config);
}

int get_port(server_config_t* config) {
    if (config == NULL) {
        return -1;
    }
    return config->port;
}

int is_port_valid(int port) {
    if (port > 0 && port < 65535) {
        return TRUE;
    }
    return FALSE;
}
