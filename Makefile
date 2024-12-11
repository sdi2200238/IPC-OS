CC = gcc
CFLAGS = -Wall -Iheaders
LDFLAGS = -lrt

SRCS = src/main.c src/sem.c src/shm.c src/input_txt.c
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))
TARGET = build/ipc_os

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf build

.PHONY: all clean