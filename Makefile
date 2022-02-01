BIN=satania
CC=gcc
CFLAGS=-O2 -I/usr/local/include
OBJ=main.o
LDFLAGS=-lX11
DEPS=Makefile

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

run: $(BIN)
	./$(BIN)
