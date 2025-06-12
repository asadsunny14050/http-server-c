#pragma once

#include <stdio.h>

#define PORT 4000
#define SERVER_NAME "cpider"
#define BUFFER_SIZE 3000
#define MAX_REQUESTS_PER_CONNECTION 20
#define SERVER_TIMEOUT 10

#define THREAD_POOL_SIZE 16

#define LOG_ERROR "31"
#define LOG_INFO "32"
#define LOG_DEBUG "33"
#define LOG_USER_INFO "34"
#define LOG_SUCCESS "35"
#define LOG_WARNING "36"

typedef struct {
  int status_code;
  char content_type[30];
  size_t content_length;
  char *connection;
  char *body;
} HttpResponse;

typedef struct {
  char *method;
  char *path;
  char *host;
  char *content_type;
  int content_length;
  char *connection;
  char *body;
} HttpRequest;
