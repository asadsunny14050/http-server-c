#pragma once

#include <stdio.h>

#define PORT 4000
#define SERVER_NAME "cpider"
#define BUFFER_SIZE 3000
#define MAX_REQUESTS_PER_CONNECTION 20
#define SERVER_TIMEOUT 10

#define THREAD_POOL_SIZE 16

typedef struct {
  char *method;
  char *path;
  char *host;
  char *connection;
  char *content_type;
  size_t content_length;
  char *body;
} HttpRequest;

typedef struct {
  int status_code;
  char *connection;
  char *content_type;
  size_t content_length;
  void *body;
} HttpResponse;

struct MimeTypeMapping {
  const char *extension;
  const char *content_type;
};
