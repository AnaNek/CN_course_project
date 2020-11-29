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

char *url_decode(const char *input);
int receive_one_chunk(int sockfd, request_t* request);
int receive_in_chunks(int sockfd, char** request, int timeout);

struct request {
    char* body;
    char* method;
    char* protocol;
    char* path;
    char* user;
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

char* get_path(request_t* request) {
    if (request == NULL) {
        return NULL;
    }
    return request->path + 1;
}

char* get_user(request_t* request) {
    if (request == NULL) {
        return NULL;
    }
    return request->user;
}

void request_free(request_t* request) {
    if (request != NULL) {
        free(request->user);
        free(request->path);
        free(request->body);
    }
    free(request);
}

int receive_one_chunk(int sockfd, request_t* request) {
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
    char* user = strstr(chunk, USER_AGENT);
    if (user == NULL) {
        free(chunk);
        return ERR_USER;
    }
    char* user_end = strstr(user, NEW_LINE);
    if (user_end == NULL) {
        free(chunk);
        return ERR_USER;
    }
    int user_len = user_end - user - strlen(USER_AGENT);
    request->user = (char*) malloc(user_len * sizeof(char));
    if (request->user == NULL) {
        free(chunk);
        return ERR_MEMORY_ALLOCATION;
    }
    strncpy(request->user, user+strlen(USER_AGENT), user_len);
    char* token = strtok(chunk, " ");
    int current = 0;
    while (token != NULL && current < 3) {
        // printf("token[%d] = %s\n", current, token);
        if (current == 0 && strcasecmp(token, GET) != 0) { // method
            free(chunk);
            return ERR_METHOD_NOT_ALLOWED;
        }

        if (current == 0) {
            request->method = token;
        }

        if (current == 1) {
            request->path = url_decode(token);
        }

        if (current == 2 && strcasecmp(token, PROTOCOL NEW_LINE HOST) != 0\
            && strcmp(token, PROTOCOL_HTTP_1_0 NEW_LINE HOST) != 0) { // protocol
            free(chunk);
            free(request->path);
            return ERR_PROTOCOL;
        }

        if (current == 2) {
            request->protocol = token;
        }

        token = strtok(NULL, " ");
        current++;
    }


    if (current < 2) {
        free(chunk);
        free(request->path);
        return ERR_METHOD_NOT_ALLOWED;
    }

    return SUCCESS;
}

int receive_in_chunks(int sockfd, char** request, int timeout) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    int total_size = 0;
    int size_recv = 0;
    int current_size = 0;
    struct timeval begin, now;
    double timediff;
    char* chunk = (char*) malloc(CHUNK_SIZE * sizeof(char));
    if (chunk == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    *request = (char *) malloc(CHUNK_SIZE * sizeof(char));
    if (*request == NULL) {
        free(chunk);
        return ERR_MEMORY_ALLOCATION;
    }
    current_size = CHUNK_SIZE;
    gettimeofday(&begin , NULL);
    while (TRUE) {
        gettimeofday(&now , NULL);
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

        if (total_size > 0 || timediff > timeout) {
            break;
        } else if (timediff > timeout * 2) {
            break;
        }

        memset(chunk, 0, CHUNK_SIZE);
        size_recv = recv(sockfd, chunk, CHUNK_SIZE, MSG_DONTWAIT);
        if (size_recv == -1) {
            usleep(100 * 1000);
        } else {
            if (total_size + size_recv > current_size) {
                char* tmp = (char*) realloc(*request, 2*current_size);
                if (tmp == NULL) {
                    free(chunk);
                    return ERR_MEMORY_ALLOCATION;
                }
                *request = tmp;
                current_size *= 2;
            }
            memcpy(*request + total_size, chunk, size_recv);
            total_size += size_recv;
            gettimeofday(&begin , NULL);
        }
    }
    return SUCCESS;
}

char *url_decode(const char *input) {
    if (input == NULL) {
        return NULL;
    }
	int input_length = strlen(input);

	size_t output_length = (input_length + 1) * sizeof(char);
	char *working = malloc(output_length), *output = working;
    if (working == NULL) {
        return NULL;
    }

	while (*input) {
		if (*input == '%') {
			char buffer[3] = { input[1], input[2], 0 };
			*working++ = strtol(buffer, NULL, 16);
			input += 3;
		}
		else {
			*working++ = *input++;
		}
	}

	*working = 0; //null terminate
	return output;
}
