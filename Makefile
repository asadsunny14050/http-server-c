CC = gcc 
CFLAGS = -Wall -Wextra

SRCS = src/main.c src/request.c src/response.c src/utils.c

TARGET = server

# all: $(TARGET)

all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)


clean: 
	rm -f $(TARGET)

