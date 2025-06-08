#include "common.h"
#include <stdio.h>

ssize_t prepare_response(HttpRequest *request, HttpResponse *response,
                         int client_fd, int client_id);
