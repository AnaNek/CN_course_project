CC = gcc
CFLAGS = -Wall -Werror
PTHREAD_LIB = -pthread
OBJS = main.o

INTERNAL_OBJS = internal/server.o internal/server_config.o internal/handler.o \
	internal/request/request.o internal/response/response.o \
	internal/context.o internal/response/filereader.o
INTERNAL = server.o server_config.o handler.o request.o response.o context.o \
	filereader.o

THREADPOOL_OBJS = threadpool/threadpool.o threadpool/task.o
THREADPOOL = threadpool.o task.o

LOGGER_OBJS = logger/stat_log.o
LOGGER = stat_log.o

THREADPOOL_TEST_OBJS = threadpool/threadpool_test.o
THREADPOOL_TEST = threadpool_test.o

ifeq ($(mode), debug)
	CFLAGS += -g3
endif

ifeq ($(mode), release)
	CFLAGS += -DNDEBUG -g0
endif

all: server.out

server.out: $(OBJS) $(INTERNAL_OBJS) $(THREADPOOL_OBJS) $(LOGGER_OBJS)
	$(CC) $(CFLAGS) -g $(PTHREAD_LIB) -o $@ $(OBJS) $(INTERNAL) $(THREADPOOL) $(LOGGER)

test_threadpool: $(THREADPOOL_TEST_OBJS) $(THREADPOOL_OBJS)
	$(CC) $(CFLAGS) -g $(PTHREAD_LIB) -o $@.out $(THREADPOOL_TEST) $(THREADPOOL)
	./$@.out

%.o: %.c *.h
	$(CC) $(CFLAGS)  -c $<
clean:
	$(RM) *.o *.out *.h.gch
