OBJS = main.o \
	   error.o \
	   pathalloc.o \

CC = gcc
CFLAGS = -Wall -g -std=c99 -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

philosopher: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

all: philosopher

clean:
	rm -f *.o myfind

.PHONY: clean all