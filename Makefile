BIN=satania
CC=gcc
CFLAGS=-O2 `pkg-config --cflags x11 cairo`
OBJ=main.o move.o mouse.o animate.o
LDFLAGS=`pkg-config --libs x11 cairo` -pthread
DEPS=Makefile

.PHONY: run

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

run: $(BIN)
	./$(BIN)
