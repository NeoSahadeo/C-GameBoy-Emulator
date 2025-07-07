CC = clang
CFLAGS = -g

SRCS = main.c utils.c cpu.c screen.c
TARGET = main

all: build-all

build-all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) -lSDL3

dev: build-all
	./$(TARGET)
