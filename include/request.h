#pragma once

#include "queue-ds.h"
#include "response.h"

int parse_headers(char *request_buffer, HttpRequest *request,
                  HttpResponse *response);

void *handle_request(Node *p_client);
