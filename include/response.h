#include "common.h"
#include <stdio.h>

ssize_t send_response(HttpRequest *request, HttpResponse *response,
                      int client_fd, int client_id);
