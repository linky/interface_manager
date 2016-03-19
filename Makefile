TARGET=interface_manager

CC=gcc
CFLAGS=-Wall -Os -lmxml -lpthread
SOURCES=if_manager.c if_private.c

all:
	$(CC) $(CFLAGS) -DNOMAIN -fPIC -shared $(SOURCES) -o $(TARGET).so
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).so
