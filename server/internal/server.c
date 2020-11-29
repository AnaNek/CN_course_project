#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "server.h"
#include "../types.h"

struct server {
    int is_running;
    int sockfd;
    struct sockaddr_in *sockaddr;
    fd_set* master_set;
    server_config_t* configuration;
};

int server_init(server_t** holder, server_config_t* config) {
    if (holder == NULL || config == NULL) {
        return ERR_PARAMS;
    }

    server_t* server = (server_t*) malloc(sizeof(server_t));
    if (server == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    server->is_running = FALSE;
    server->sockfd = -1;
    server->sockaddr = NULL;
    server->master_set = (fd_set *) malloc(sizeof(fd_set));
    if (server->master_set == NULL) {
        free(server);
        return ERR_MEMORY_ALLOCATION;
    }
    FD_ZERO(server->master_set);
    server->configuration = config;
    *holder = server;
    return SUCCESS;
}


void server_free(server_t* server) {
    if (server != NULL) {
        close(server->sockfd);
        free(server->master_set);
        free(server->sockaddr);
    }
    free(server);
}

int is_running(server_t* s) {
    if (s == NULL) {
        return FALSE;
    }

    if (s->is_running == TRUE) {
        return TRUE;
    }
    return FALSE;
}

int set_socket(server_t* s, int sockfd) {
    if (s == NULL) {
        return ERR_PARAMS;
    }
    s->sockfd = sockfd;
    return SUCCESS;
}

int server_run(server_t* s) {
    if (s == NULL || s->configuration == NULL || s->is_running == TRUE) {
        return ERR_PARAMS;
    }
    int port = get_port(s->configuration);

    if (port == -1) {
        return ERR_INVALID_PORT;
    }

    int rc = SUCCESS;
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        return ERR_SOCKET;
    }
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    rc = set_socket(s, sockfd);
    if (rc != SUCCESS) {
        return ERR_SOCKET;
    }

    FD_SET(sockfd, s->master_set);

    struct sockaddr_in *serv_addr =(struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    if (serv_addr == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr->sin_port = htons(port);
    s->sockaddr = serv_addr;

    int enable = 1;
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rc == -1) {
        return ERR_SOCKOPT;
    }

    rc = bind(sockfd, (struct sockaddr *) serv_addr, sizeof(struct sockaddr_in));
    if (rc == -1) {
        return ERR_BIND;
    }

    rc = listen(sockfd, 4096);
    if (rc == -1) {
        return ERR_LISTEN;
    }

    s->is_running = TRUE;
    return SUCCESS;
}

int server_stop(server_t* s) {
    if (s == NULL) {
        return ERR_PARAMS;
    }

    if (s->is_running == FALSE) {
        return SUCCESS;
    }

    int rc = close(s->sockfd);
    if (rc == -1) {
        s->is_running = FALSE;
        return ERR_CLOSE;
    }
    s->is_running = FALSE;
    return SUCCESS;
}

int server_select(server_t* s) {
    if (s == NULL) {
        return ERR_PARAMS;
    }

    int rc = select(s->sockfd+1, s->master_set, NULL, NULL, NULL);

    if (rc == -1) {
        return ERR_SELECT;
    }

    if (rc == 0) {
        return ERR_SELECT_TIMEOUT;
    }

    if (FD_ISSET(s->sockfd, s->master_set)) {
        return SUCCESS;
    }
    return ERR_SELECT_SOCK;
}

int server_accept(server_t* s, int* accepted_sockfd) {
    if (s == NULL || accepted_sockfd == NULL) {
        return ERR_PARAMS;
    }

    socklen_t len = sizeof(struct sockaddr_in);

    int accepted = accept(s->sockfd, (struct sockaddr *) s->sockaddr, &len);

    if (accepted == -1) {
        return ERR_ACCEPT;
    }

    *accepted_sockfd = accepted;
    return SUCCESS;
}
