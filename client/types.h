#ifndef __TYPES_H__
#define __TYPES_H__

#define ARGC 4
#define THREAD_MAX 256

#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define ERR_MEMORY_ALLOCATION (-1)
#define ERR_PARAMS (-2)
#define ERR_FILE_NOT_EXIST (-3)
#define ERR_INVALID_PORT (-4)
#define ERR_SOCKET (-5)
#define ERR_BIND (-6)
#define ERR_LISTEN (-7)
#define ERR_CLOSE (-8)
#define ERR_MUTEX_LOCK (-9)
#define ERR_MUTEX_UNLOCK (-10)
#define ERR_ACCEPT (-11)
#define ERR_CREATE_THREAD (-12)
#define ERR_RECV (-13)
#define ERR_SOCKOPT (-14)
#define ERR_METHOD_NOT_ALLOWED (-15)
#define ERR_PROTOCOL (-16)
#define ERR_FCNTL (-17)
#define ERR_EPOLL (-18)
#define ERR_EPOLL_CTL (-19)
#define ERR_POLL (-20)
#define ERR_POLL_TIMEOUT (-21)
#define ERR_SELECT (-22)
#define ERR_SELECT_TIMEOUT (-23)
#define ERR_SELECT_SOCK (-24)
#define ERR_PATH (-25)
#define ERR_STAT (-26)
#define ERR_OPEN (-27)
#define ERR_USER (-28)

#define GET "GET"
#define PROTOCOL "HTTP/1.1"
#define PROTOCOL_HTTP_1_0 "HTTP/1.0"
#define NEW_LINE "\r\n"
#define HOST "Host:"
#define USER_AGENT "User-Agent:"
#define CONTENT_LENGTH "Content-Length:"

#endif
