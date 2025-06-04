#pragma once

#include "queue-ds.h"

typedef struct {
  char *method;
  char *path;
} HttpRequest;

void parse_headers(char *request_buffer, HttpRequest *request,
                   int buffer_length);

void *handle_request(Node *p_client);
