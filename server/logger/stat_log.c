#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "stat_log.h"
#include "../types.h"


int open_log_file(int* logfd) {
    if (logfd == NULL) {
        return ERR_PARAMS;
    }

    int fd = open("files.log", O_CREAT|O_RDWR|O_APPEND);
    if (fd == -1) {
        return ERR_OPEN;
    }
    *logfd = fd;
    return SUCCESS;
}

char* get_ext(char* path) {
    if (path == NULL) {
        return "";
    }

    char* find_dot = strrchr(path, '.');
    if (find_dot == NULL) {
        return "";
    }
    return find_dot + 1;
}

int write_statistic(int logfd, char* path, char* user) {
    if (logfd == -1 || path == NULL || user == NULL) {
        return ERR_PARAMS;
    }

    char* buffer = (char*) malloc(LOG_BUFFER * sizeof(char));
    if (buffer == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }


    sprintf(buffer, "User #{%s} requested file ext {%s}\n", user, get_ext(path));
    write(logfd, buffer, strlen(buffer));
    return SUCCESS;
}
