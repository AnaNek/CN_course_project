CC = gcc
CFLAGS = -g
OBJS = client.o

ifeq ($(mode), debug)
	CFLAGS += -g3
endif

ifeq ($(mode), release)
	CFLAGS += -DNDEBUG -g0
endif

all: client

client: $(OBJS)
	$(CC) $(CFLAGS) -o client $^

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) *.o *.out *.h.gch
