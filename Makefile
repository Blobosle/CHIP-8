CC = gcc
CFLAGS = -Wall -arch arm64 $(shell sdl2-config --cflags)
LDFLAGS = -arch arm64 $(shell sdl2-config --libs)
SRC = chip8.c main.c display.c
TEST_SRC = chip8.c tester.c
OBJ = $(SRC:.c=.o)
TEST_OBJ = $(TEST_SRC:.c=.o)
TARGET = emu

.PHONY: all test clean

all: $(OBJ)
	$(CC) $(LDFLAGS) -o $(TARGET) $^

test: $(TEST_OBJ)
	tr -d ' \n' < bytes.hex | xxd -r -p > test.bin
	$(CC) -o emu_test $^

clean:
	rm -f $(OBJ) emu_test
