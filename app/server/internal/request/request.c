#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>


#include "request.h"
#include "../../types.h"

//char *url_decode(const char *input);
int receive_one_chunk(int sockfd, request_t* request);
int receive_in_chunks(int sockfd, char** request, int timeout);
int check_pid(char *token, int *pid);

struct request {
    int   pid;
    char* body;
    char* method;
    char* protocol;
};

int request_parse(request_t** holder, int client_sockfd) {
    if (holder == NULL) {
        return ERR_PARAMS;
    }

    request_t* request = (request_t*) malloc(sizeof(request_t));
    if (request == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    int rc = receive_one_chunk(client_sockfd, request);
    if (rc != SUCCESS) {
        free(request);
        return rc;
    }
    *holder = request;
    return SUCCESS;
}

int get_pid(request_t* request) {
    if (request == NULL) {
        return -1;
    }
    return request->pid;
}

char* get_method(request_t* request) {
    if (request == NULL) {
        return NULL;
    }
    return request->method;
}

void request_free(request_t* request) {
    if (request != NULL) {
        free(request->body);
    }
    free(request);
}

int receive_one_chunk(int sockfd, request_t* request) {
    int rc = 0;
    int pid = 0;
    char* chunk = (char*) malloc(CHUNK_SIZE * sizeof(char));
    if (chunk == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    int size_recv = recv(sockfd, chunk, CHUNK_SIZE, 0);
    if (size_recv == -1) {
        free(chunk);
        return ERR_RECV;
    }
    request->body = chunk;
    printf("[%d] Request size: %d, body:  %s\n", sockfd, size_recv, request->body);

    char* token = strtok(chunk, " ");
    int current = 0;
    while (token != NULL) {
        if (current == 0 && strcasecmp(token, SHOWALL) != 0 &&
            strcasecmp(token, GET) != 0 && strcasecmp(token, ADD) != 0 &&
            strcasecmp(token, REMOVE) != 0) { // method
            free(chunk);
            return ERR_METHOD_NOT_ALLOWED;
        }

        if (current == 0) {
            request->method = token;
        }

        if (current == 1 && strcmp(token, MY_PROTOCOL) != 0) { // protocol
            free(chunk);
            return ERR_PROTOCOL;
        }

        if (current == 1) {
            request->protocol = token;
        }

        if (current == 2) {
            rc = check_pid(token, &pid);

            if (rc && strcasecmp(request->method, SHOWALL) != 0)
                return rc;

            if (rc != ERR_REQUEST && strcasecmp(request->method, SHOWALL) == 0)
                return ERR_REQUEST;

            if (strcasecmp(request->method, SHOWALL) == 0)
            {
                pid = 0;
            }

            request->pid = pid;
        }

        //printf("token %s\n", token);

        token = strtok(NULL, " ");
        current++;
    }


    if (current < 3 && strcasecmp(request->method, SHOWALL) != 0) {
        free(chunk);
        return ERR_REQUEST;
    }

    if (current < 2 && strcasecmp(request->method, SHOWALL) == 0) {
        free(chunk);
        return ERR_REQUEST;
    }

    //printf("current %d\n", current);
    if (current > 3) {
        free(chunk);
        return ERR_REQUEST;
    }

    return SUCCESS;
}

int check_pid(char *token, int *pid)
{
    char *end = NULL;

    if (strlen(token) == 0)
        return ERR_REQUEST;

    if (*token == '\n' || *token == '\0' || *token == '\r')
        return ERR_REQUEST;

    long int res = strtol(token, &end, 10);

    if ((*end != '\0') && (*end != '\n') && (*end != '\r'))
        return ERR_PID;

    if (res < 0)
        return ERR_PID;

    *pid = res;
    return SUCCESS;
}
