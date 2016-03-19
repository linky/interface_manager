TARGET=interface_manager

CC=gcc
CFLAGS=-Wall -Os -lmxml -lpthread
SOURCES=bind.c lshw.c

all:
	$(CC) $(CFLAGS) -DNOMAIN -fPIC -shared $(SOURCES) -o $(TARGET).so
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f bind bind.so
