CC = gcc
CFLAGS = -Wall -g
TARGET = peer
SRCS = P2P.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) *.o