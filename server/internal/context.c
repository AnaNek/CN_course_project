#include <stdlib.h>

#include "context.h"
#include "../types.h"

struct context {
    int sockfd;
    int logfd;
    const char* base;
};

int context_init(context_t** holder, int sockfd, const char* base) {
    if (holder == NULL || base == NULL) {
        return ERR_PARAMS;
    }

    context_t* context = (context_t*) malloc(sizeof(context_t));
    if (context == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    context->sockfd = sockfd;
    context->logfd = -1;
    context->base = base;
    *holder = context;
    return SUCCESS;
}

int get_sockfd(context_t* context) {
    if (context == NULL) {
        return ERR_PARAMS;
    }
    return context->sockfd;
}

int set_logfd(context_t* context, int logfd) {
    if (context == NULL) {
        return ERR_PARAMS;
    }
    context->logfd = logfd;
    return SUCCESS;
}

int get_logfd(context_t* context) {
    if (context == NULL) {
        return ERR_PARAMS;
    }
    return context->logfd;
}

const char* get_base(context_t* context) {
    if (context == NULL) {
        return NULL;
    }
    return context->base;
}

void context_free(context_t* context) {
    if (context == NULL) {
        return;
    }
    free(context);
}
