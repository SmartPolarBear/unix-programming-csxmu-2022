
OUT = build
SRC = .
CC = gcc
LD = ld

all: timewrite

${OUT}: 
	mkdir -p $(OUT)

${OUT}/timewrite.o: ${OUT} $(SRC)/timewrite.c
	$(CC) -c $(SRC)/timewrite.c -o $(OUT)/timewrite.o

timewrite: $(OUT)/timewrite.o
	$(CC) $(OUT)/timewrite.o -o timewrite

clean: 
	rm -rf $(OUT) timewrite

.PHONY: all clean
