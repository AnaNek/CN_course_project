#ifndef __REQUEST_H__
#define __REQUEST_H__

#define CHUNK_SIZE 4096

typedef struct request request_t;

int request_parse(request_t** holder, int client_sockfd);

void request_free(request_t* request);

int get_pid(request_t* request);

char* get_method(request_t* request);

char* get_user(request_t* request);

#endif
