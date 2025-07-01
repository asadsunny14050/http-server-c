CC = gcc 
CFLAGS = -Wall -Wextra -g

SRCS = src/main.c src/request.c src/response.c src/queue-ds.c src/utils.c

TARGET = cpider

all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -lz -o $(TARGET)


clean: 
	rm -f $(TARGET)

