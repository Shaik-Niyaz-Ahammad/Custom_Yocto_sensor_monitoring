CC ?= gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = sensor_app

SRC = src/main.c src/bme280.c src/config.c 

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
