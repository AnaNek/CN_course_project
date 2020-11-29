#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "types.h"
#define CHUNK_SIZE 4096
#define MAX_REQUEST_SIZE 256
#define PORT 8080
#define SA struct sockaddr

#define REQUEST "GET %s HTTP/1.1\r\n\
Host: localhost:%d\r\n\
User-Agent: %s\r\n\
Accept: */*\r\n\r\n"

int flag = 0;

void handle_interrupt(int sig_num) {
    printf("Interrupt: %d\n", sig_num);
    flag = 1;
	exit(0);
}

int receive_in_chunks(int sockfd, char** response, int timeout) {
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

    *response = (char *) malloc(CHUNK_SIZE * sizeof(char));
    if (*response == NULL) {
        free(chunk);
        return ERR_MEMORY_ALLOCATION;
    }
    current_size = CHUNK_SIZE;
    gettimeofday(&begin , NULL);
	int count_chunks = 0;
	int content_length = 0;
    while (count_chunks <= 1 || total_size < content_length) {
        gettimeofday(&now , NULL);
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

        if (timediff > timeout) {
            break;
        } else if (timediff > timeout * 2) {
            break;
        }

        memset(chunk, 0, CHUNK_SIZE);
        size_recv = recv(sockfd, chunk, CHUNK_SIZE, MSG_DONTWAIT);
        if (size_recv == -1) {
            usleep(100 * 1000);
        } else {
			count_chunks++;
			if (count_chunks == 1) {
				char *cl_beg = strstr(chunk, CONTENT_LENGTH);
				if (cl_beg == NULL) {
					free(chunk);
					return ERR_PARAMS;
				}
				char* cl_end = strstr(cl_beg, NEW_LINE);
			    if (cl_end == NULL) {
			        free(chunk);
			        return ERR_PARAMS;
			    }
				content_length = atoi(cl_beg + strlen(CONTENT_LENGTH));
			}
            if (total_size + size_recv > current_size) {
                char* tmp = (char*) realloc(*response, 2*current_size);
                if (tmp == NULL) {
                    free(chunk);
                    return ERR_MEMORY_ALLOCATION;
                }
                *response = tmp;
                current_size *= 2;
            }
            memcpy(*response + total_size, chunk, size_recv);
            total_size += size_recv;
            gettimeofday(&begin , NULL);
        }
    }
    return SUCCESS;
}

int main(int argc, char* argv[]) {
	int sockfd, connfd;
	struct sockaddr_in servaddr;
	signal(SIGINT, handle_interrupt);
	printf("Ctrl+C to exit established\n");

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}

	int enable = 1;
    int rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (rc == -1) {
        return ERR_SOCKOPT;
    }

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	char* user_agent = NULL;
	char* path = NULL;
	size_t ualen = 0;
	size_t plen = 0;
	int read = 0;
	int sent = 0;

	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		printf("Errno state: %s\n", strerror(errno));
		exit(0);
	}

	printf("Input relative pathname starting with slash - '/'\n");
	if ((read = getline(&path, &plen, stdin)) == -1) {
		free(path);
		close(sockfd);
		printf("Read pathname failed\n");
		exit(0);
	}
	int len = strlen(path);
	if (path[len - 1] == '\n') {
		path[len - 1] = '\0';
	}
	printf("Input User-Agent header value\n");
	if ((read = getline(&user_agent, &ualen, stdin)) == -1) {
		free(path);
		free(user_agent);
		close(sockfd);
		printf("Read UA failed\n");
		exit(0);
	}
	len = strlen(user_agent);
	if (user_agent[len - 1] == '\n') {
		user_agent[len - 1] = '\0';
	}

	char* request = (char*) malloc(MAX_REQUEST_SIZE);
	if (request == NULL) {
		free(path);
		free(user_agent);
		close(sockfd);
		printf("Malloc failed\n");
		exit(0);
	}

	sprintf(request, REQUEST, path, PORT, user_agent);
	sent = send(sockfd, request, strlen(request), 0);
	printf("Request sent, bytes sent: %d\n", sent);
	free(path);
	path = NULL;
	free(user_agent);
	user_agent = NULL;
	free(request);
	request = NULL;

	char* response = NULL;
	rc = receive_in_chunks(sockfd, &response, 1);
	printf("Response rc = %d\n", rc);
	if (rc != SUCCESS) {
		response = NULL;
		close(sockfd);
		printf("Failed\n");
		exit(0);
	}
	printf("Got response:\n%s\n", response);
	free(response);

	close(sockfd);
	return 0;
}
