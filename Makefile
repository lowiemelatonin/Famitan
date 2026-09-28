CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET = nes
SRC = *.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)