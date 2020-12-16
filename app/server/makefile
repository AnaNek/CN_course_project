CC = gcc
CFLAGS = -Wall -Werror
PTHREAD_LIB = -pthread
OBJS = main.o

INTERNAL_OBJS = internal/server.o internal/server_config.o internal/handler.o \
	internal/request/request.o internal/response/response.o 
INTERNAL = server.o server_config.o handler.o request.o response.o

THREADPOOL_OBJS = threadpool/threadpool.o threadpool/task.o
THREADPOOL = threadpool.o task.o

ifeq ($(mode), debug)
	CFLAGS += -g3
endif

ifeq ($(mode), release)
	CFLAGS += -DNDEBUG -g0
endif

all: server

server: $(OBJS) $(INTERNAL_OBJS) $(THREADPOOL_OBJS)
	$(CC) $(CFLAGS) -g $(PTHREAD_LIB) -o $@ $(OBJS) $(INTERNAL) $(THREADPOOL)

%.o: %.c *.h
	$(CC) $(CFLAGS)  -c $<
clean:
	$(RM) *.o *.out *.h.gch
