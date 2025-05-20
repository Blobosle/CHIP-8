CC := gcc
CFLAGS := -Wall
SRC := chip8.c tester.c
OBJ := $(SRC:.c=.o)
TARGET := emu

.PHONY: all test clean

all: $(OBJ)
	$(CC) -o $(TARGET) $^

test:
	tr -d ' \n' < bytes.hex | xxd -r -p > test.bin

clean:
	rm -f $(OBJ)
