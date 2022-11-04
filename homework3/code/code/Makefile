OBJS = main.o \
	   error.o \
	   pathalloc.o \
	   parser.o \
	   scanner.o \
	   findcmd.o \

CC = gcc
CFLAGS = -Wall -g -std=c99

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

myshell: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

all: myshell

clean:
	rm -f *.o myshell

.PHONY: clean all