CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = smart_parking.exe

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) -lm

clean:
	rm -f $(TARGET)

demo: $(TARGET)
	./$(TARGET) --demo
