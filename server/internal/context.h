#ifndef __CONTEXT_H__
#define __CONTEXT_H__

typedef struct context context_t;

int context_init(context_t** holder, int sockfd, const char* base);

int get_sockfd(context_t* context);

const char* get_base(context_t* context);

void context_free(context_t* context);

int set_logfd(context_t* context, int logfd);

int get_logfd(context_t* context);

#endif
