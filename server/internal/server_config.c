#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "server_config.h"
#include "../types.h"

struct config {
    int port;
    int threads_limit;
    const char *document_root;
};

// PRIVATE DECLARE
int is_document_root_valid(const char *document_root);
int is_port_valid(int port);

// PUBLIC DEFINE
int server_config_init(
    server_config_t** holder,
    int port,
    int threads_limit,
    const char* document_root) {
    if (holder == NULL || document_root == NULL) {
        return ERR_PARAMS;
    }

    if (is_port_valid(port) != TRUE) {
        return ERR_INVALID_PORT;
    }

    if (is_document_root_valid(document_root) != TRUE) {
        return ERR_FILE_NOT_EXIST;
    }

    server_config_t* config = (server_config_t*) malloc(sizeof(server_config_t));
    if (config == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    config->port = port;
    config->threads_limit = threads_limit;
    config->document_root = document_root;

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

const char* get_root(server_config_t* config) {
    if (config == NULL) {
        return NULL;
    }
    return config->document_root;
}

int is_document_root_valid(const char *document_root) {
    struct stat stats;
    int rc = stat(document_root, &stats);
    if (rc == -1) {
        return FALSE;
    }

    if (S_ISDIR(stats.st_mode)) {
        return TRUE;
    }

    return FALSE;
}

int is_port_valid(int port) {
    if (port > 0 && port < 65535) {
        return TRUE;
    }
    return FALSE;
}
