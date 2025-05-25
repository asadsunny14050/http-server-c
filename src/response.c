#include "../include/response.h"
#include "../include/request.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "../include/common.h"

int read_html_file(HttpRequest *request, HttpResponse *response,
                   int client_id) {

  char file_path[30];

  if (strcmp(request->path, "/home") == 0 || strcmp(request->path, "/") == 0) {
    strncpy(file_path, "./assets/index.html", 30);
  } else {
    snprintf(file_path, 30, "./assets%s.html", request->path);
  }

  FILE *file_to_read = fopen(file_path, "r");
  log_to_debug(&logs.debug, "Requested Resource Path: %s", file_path,
               client_id);

  if (!file_to_read) {
    log_to_console(&logs.error, "Failed to load the file that has been given",
                   0, client_id);
    response->status_code = 500;
    return -1;
  }

  log_to_console(&logs.user, "Requested HTML file opened, sire!", 0, client_id);
  struct stat file_statbuf;
  if (fstat(fileno(file_to_read), &file_statbuf) != 0) {
    log_to_console(&logs.error,
                   "Failed to get the file information of the file", 0,
                   client_id);
    fclose(file_to_read);
    response->status_code = 500;
    return -1;
  }

  size_t required_file_size = file_statbuf.st_size;
  // remember to free this
  char *response_buffer = (char *)malloc(required_file_size);

  size_t bytes_read =
      fread(response_buffer, 1, required_file_size, file_to_read);
  if (bytes_read != required_file_size) {
    log_to_console(&logs.error, "Read Operation faild, sire!", 0, client_id);
    fclose(file_to_read);
    response->status_code = 500;
    return -1;
  }
  response->body = response_buffer;
  response->body[required_file_size] = '\0';
  log_to_console(&logs.user, "File Read is successful, sire!", 0, client_id);
  return 0;
}

ssize_t prepare_response(HttpRequest *request, HttpResponse *response,
                         int client_fd, int client_id) {

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
      if (read_html_file(request, response, client_id) == 0) {
        response->content_length = strlen(response->body);
      }
      break;
    }
  }

  if (response->status_code == 401) {
    char *error_body = (char *)malloc(BUFFER_SIZE);
    strncpy(error_body,
            "<html><body><h1>404, Page not found!</h1></body></html>",
            BUFFER_SIZE);
    response->body = error_body;
    response->content_length = strlen(response->body);
  }

  if (response->status_code == 500) {
    char *error_body = (char *)malloc(BUFFER_SIZE);
    strncpy(error_body,
            "<html><body><h1>Internal Server Error!</h1></body></html>",
            BUFFER_SIZE);
    response->body = error_body;
    response->content_length = strlen(response->body);
  }

  char response_buffer[BUFFER_SIZE + sizeof(response->body)];
  snprintf(response_buffer, (BUFFER_SIZE + sizeof(response->body)),
           "HTTP/1.1 %d OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %zu\r\n"
           "Server: %s\r\n"
           "\r\n"
           "%s",
           response->status_code, response->content_type,
           response->content_length, SERVER_NAME, response->body);

  free(response->body);
  return send(client_fd, response_buffer, strlen(response_buffer), 0);
}
