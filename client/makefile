CC = gcc
CFLAGS = -g
OBJS = main.o

ifeq ($(mode), debug)
	CFLAGS += -g3
endif

ifeq ($(mode), release)
	CFLAGS += -DNDEBUG -g0
endif

all: client.out

client.out: $(OBJS)
	$(CC) $(CFLAGS) -o client.out $^

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) *.o *.out *.h.gch
