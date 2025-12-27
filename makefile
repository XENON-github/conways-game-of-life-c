CC = gcc
TARGET = conway
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	sudo rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)
