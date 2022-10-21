OBJS = main.o \
	   error.o \
	   pathalloc.o \

CC = gcc
CFLAGS = -Wall -g -std=c99

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

myfind: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o myfind

all: myfind

clean:
	rm -f *.o myfind

.PHONY: clean all