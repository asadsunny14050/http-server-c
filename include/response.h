#include "common.h"
#include "request.h"
#include <stdio.h>

typedef struct {
  int status_code;
  char content_type[30];
  size_t content_length;
  char body[BUFFER_SIZE];
} HttpResponse;

ssize_t prepare_response(HttpRequest *request, HttpResponse *response,
                         int client_fd);
