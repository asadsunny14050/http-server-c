#include "../include/response.h"
#include "../include/request.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "../include/common.h"

ssize_t prepare_response(HttpRequest *request, HttpResponse *response,
                         int client_fd) {

  char *requested_path = request->path;
  char *requested_method = request->method;
  response->status_code = 401;
  strncpy(response->content_type, "text/html", 30);

  char *routes[] = {
      "/",
      "/home",
      "/about",
      "/contact",
  };

  int amt_of_routes = sizeof(routes) / sizeof(routes[0]);

  for (int i = 0; i < amt_of_routes; i++) {
    if (strcmp(requested_path, routes[i]) == 0 &&
        strcmp(requested_method, "GET") == 0) {
      response->status_code = 200;
      snprintf(response->body, BUFFER_SIZE,
               "<html><body><h1>This is the %s Page</h1></body></html>",
               routes[i]);
      response->content_length = strlen(response->body);
      break;
    }
  }

  if (response->status_code == 401) {
    strncpy(response->body,
            "<html><body><h1>404, Page not found!</h1></body></html>",
            BUFFER_SIZE);
    response->content_length = strlen(response->body);
  }

  char response_buffer[2 * BUFFER_SIZE];
  snprintf(response_buffer, (2 * BUFFER_SIZE),
           "HTTP/1.1 %d OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %zu\r\n"
           "Server: %s\r\n"
           "\r\n"
           "%s",
           response->status_code, response->content_type,
           response->content_length, SERVER, response->body);

  return send(client_fd, response_buffer, strlen(response_buffer), 0);
}
