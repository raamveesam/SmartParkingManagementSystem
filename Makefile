CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRC = src/main.c src/utils.c src/vehicle_registry.c src/slot_manager.c \
      src/waiting_queue.c src/billing.c src/entry_exit.c src/search_sort.c \
      src/ai_predictor.c src/statistics.c src/file_io.c src/demo.c
TARGET = smart_parking.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

demo: $(TARGET)
	./$(TARGET) --demo
