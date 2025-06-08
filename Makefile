CC = gcc 
CFLAGS = -Wall -Wextra -g

SRCS = src/main.c src/request.c src/response.c src/queue-ds.c src/utils.c

TARGET = cpider-web-server

all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)


clean: 
	rm -f $(TARGET)

