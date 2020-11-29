#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "context.h"
#include "request/request.h"
#include "response/response.h"
#include "handler.h"
#include "../logger/stat_log.h"
#include "../types.h"

char* make_path(char* path);
int check_path(char* path);

void handle_new_connection(void* arg) {
    context_t* context = (context_t *) arg;
    int client_sockfd = get_sockfd(context);
    int logfd = get_logfd(context);
    int rc = SUCCESS;
    request_t* request = NULL;
    rc = request_parse(&request, client_sockfd);
    printf("[%d] Request parse finished with status %d\n", client_sockfd, rc);

    if (rc != SUCCESS) {
        if (rc == ERR_USER) {
            send_error_response(client_sockfd, STATUS_UNAUTHORIZED);
        } else {
            send_error_response(client_sockfd, STATUS_BAD_REQUEST);
        }
        context_free(context);
        close(client_sockfd);
        printf("Couldn't parse request with status: %d\n", rc);
        return;
    }

    char* path = get_path(request);
    if (check_path(path) != TRUE) {
        send_error_response(client_sockfd, STATUS_FORBIDDEN);
        context_free(context);
        request_free(request);
        close(client_sockfd);
        printf("Couldn't access file\n");
        return;
    }

    write_statistic(logfd, path, get_user(request));
    send_response(client_sockfd, path);
    context_free(context);
    request_free(request);
    close(client_sockfd);
}

char* make_path(char* path) {
    if (path == NULL) {
        return NULL;
    }

    int len_path = strlen(path);
    int whole = len_path;
    char* dest = (char*) malloc(whole * sizeof(char));
    if (dest == NULL) {
        return NULL;
    }
    strcpy(dest, path + 1);

    return dest;
}

int check_path(char* path) {
    if (path == NULL) {
        return FALSE;
    }

    if (strstr(path, "/..") != NULL) {
        return FALSE;
    }

    if (access(path, R_OK) != -1) {
        return TRUE;
    }

    return FALSE;
}
