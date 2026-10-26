CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = friendrec

SRCS = main.c common.c hashtable.c queue.c graph.c storage.c
OBJS = $(SRCS:.c=.o)
HEADERS = common.h hashtable.h queue.h graph.h storage.h

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
