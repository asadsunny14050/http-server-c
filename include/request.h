#pragma once

typedef struct {
  char *method;
  char *path;
} HttpRequest;

void parse_headers(char *request_buffer, HttpRequest *request,
                   int buffer_length);

void *handle_request(void *p_client_fd);
