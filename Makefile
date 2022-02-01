BIN=satania
CC=gcc
CFLAGS=-O2 `pkg-config --cflags x11 cairo pthread-stubs`
OBJ=main.o
LDFLAGS=`pkg-config --libs x11 cairo pthread-stubs`
DEPS=Makefile

all: $(BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

run: $(BIN)
	./$(BIN)
