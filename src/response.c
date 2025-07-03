#include "../include/response.h"
#include "../include/request.h"
#include "../include/utils.h"

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "../include/common.h"

const struct MimeTypeMapping mime_types[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"txt", "text/plain"},
    {"csv", "text/csv"},
    {"xml", "application/xml"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"gif", "image/gif"},
    {"webp", "image/webp"},
    {"svg", "image/svg+xml"},
    {"ico", "image/x-icon"},
    {"mp4", "video/mp4"},
    {"webm", "video/webm"},
    {"ogg", "video/ogg"},
    {"mp3", "audio/mpeg"},
    {"wav", "audio/wav"},
    {NULL, NULL} // Sentinel to mark end of array
};

int match_and_set_content_type(char *file_extension, HttpResponse *response) {

  int result = -1;
  for (int i = 0; mime_types[i].extension != NULL; i++) {
    result = strcmp(mime_types[i].extension, file_extension);
    if (result == 0) {
      response->content_type = (char *)mime_types[i].content_type;
      break;
    }
  }

  return result;
}

static const char *http_status_messages[] = {"OK",
                                             "Created",
                                             "Bad Request",
                                             "Unauthorized",
                                             "Forbidden",
                                             "Not Found",
                                             "Unsupported Media Type",
                                             "Internal Server Error"};

static const char *get_http_message(int status_code) {
  switch (status_code) {
  case 200:
    return http_status_messages[0];
    break;
  case 201:
    return http_status_messages[1];
    break;
  case 400:
    return http_status_messages[2];
    break;
  case 401:
    return http_status_messages[3];
    break;
  case 403:
    return http_status_messages[4];
    break;
  case 404:
    return http_status_messages[5];
    break;
  case 415:
    return http_status_messages[6];
    break;
  case 500:
    return http_status_messages[7];
    break;
  default:
    return http_status_messages[0];
  }
}

FILE *open_static_file(HttpRequest *request, HttpResponse *response, int client_id) {

  extern char *public_directory;

  char file_path[50];

  if (strcmp(request->path, "/home") == 0 || strcmp(request->path, "/") == 0) {

    if (request->accept_encoding == NULL) {

      snprintf(file_path, 50, "%s%s", public_directory, "/index.html");
    } else {

      snprintf(file_path, 50, "%s%s", public_directory, "/compressed/index.html.gz");
    }

  } else if (strchr(request->path, '.') == NULL) {
    if (request->accept_encoding == NULL) {

      snprintf(file_path, 50, "%s%s.html", public_directory, request->path);
    } else {

      snprintf(file_path, 50, "%s/compressed/%s.html.gz", public_directory, request->path);
    }

  } else {

    char *file_extension = strchr(request->path, '.') + 1;
    if (file_extension == NULL) {
      log_to_console(&logs.error, "No File Extension, sire!", 0, client_id);
      response->status_code = 500;
      return NULL;
    }
    if (match_and_set_content_type(file_extension, response) != 0) {
      response->status_code = 415;
      return NULL;
    };

    bool is_text_file = (strstr(response->content_type, "text") != NULL) || (strstr(response->content_type, "application") != NULL);
    if (request->accept_encoding != NULL && is_text_file) {

      snprintf(file_path, 50, "%s/compressed/%s.gz", public_directory, request->path);
    } else {
      snprintf(file_path, 50, "%s%s", public_directory, request->path);
    }
  }

  FILE *file_to_read = fopen(file_path, "rb");

  if (file_to_read == NULL) {
    log_to_console(&logs.error, "The File doesn't exist, sire!", 0, client_id);
    response->status_code = 404;
    return NULL;
  }
  log_to_debug(&logs.debug, "Actual Resource Path found: %s", file_path, client_id);
  log_to_console(&logs.user, "Requested Static file opened, sire!", 0, client_id);
  struct stat file_statbuf;
  int stat_result = fstat(fileno(file_to_read), &file_statbuf);

  if (stat_result < 0) {
    log_to_console(&logs.error, "Failed to get the file information of the file", 0, client_id);
    fclose(file_to_read);
    response->status_code = 500;
    return NULL;
  }

  if (file_statbuf.st_size > MAX_FILE_SIZE) {
    log_to_console(&logs.error, "File exceeds the max size Cpider is set to serve", 0, client_id);
    fclose(file_to_read);
    response->status_code = 403;
    return NULL;
  }

  response->content_length = file_statbuf.st_size;
  return file_to_read;
}

void handle_post(HttpRequest *request, HttpResponse *response, int client_id) {

  char *token;
  unsigned long insert_buffer_size = 2 * strlen(request->body);
  char insert_buffer[insert_buffer_size];
  memset(&insert_buffer, 0, insert_buffer_size);
  char *body_start = request->body;
  while ((token = strtok_r(body_start, "&", &body_start)) != NULL) {
    if (insert_buffer[0] == 0) {
      strncpy(insert_buffer, token, strlen(token));
    } else {
      strcat(insert_buffer, ", ");
      strcat(insert_buffer, token);
    }
  }
  strcat(insert_buffer, "\n");

  FILE *file_to_write_to = fopen("./records.txt", "a");
  if (file_to_write_to == NULL) {
    log_to_console(&logs.error, "Our Records file is missing, sire!", 0,
                   client_id);
    response->status_code = 500;
  }

  if (fprintf(file_to_write_to, "%s", insert_buffer) < 0) {
    log_to_console(&logs.error, "Failed to add to the record, sire!", 0,
                   client_id);
    printf("failed write operation, sire\n");
    response->status_code = 500;
  }

  log_to_console(&logs.info, "Record added, sire!", 0, client_id);
  fclose(file_to_write_to);
}

ssize_t send_response(HttpRequest *request, HttpResponse *response, int client_fd, int client_id) {

  char *requested_path = request->path;
  char *requested_method = request->method;
  response->content_type = (char *)mime_types[0].content_type;
  FILE *file_to_send;
  if (response->status_code != 400 && strcmp(requested_method, "GET") == 0) {
    response->status_code = 404;
    log_to_debug(&logs.user, "Requested Path: %s", requested_path, client_id);
    file_to_send = open_static_file(request, response, client_id);
    if (file_to_send != NULL) {

      response->status_code = 200;
    }
  }

  if (response->status_code == 201) {
    handle_post(request, response, client_id);
  }

  const char *http_message = get_http_message(response->status_code);

  if (strcmp(request->path, "/home") == 0 || strcmp(request->path, "/") == 0) {
    response->status_code = 200;
    response->body = "<html><body>"
                     "<h1>Welcome to Cpider!</h1>"
                     "<h2>Your Friendly Neighborhood Http Server!</h2>"
                     "<p>Create an index.html file to replace this as your Home Page</p>"
                     "</body></html>";
    response->content_length = strlen(response->body);
  }

  if (response->status_code != 200) {
    response->body = (char *)malloc(BUFFER_SIZE);
    snprintf(response->body, BUFFER_SIZE,
             "<html><body><h1>%d, %s</h1></body></html>", response->status_code,
             http_message);
    response->content_length = strlen(response->body);
  }

  if (request->connection != NULL && strcmp("close", request->connection) == 0) {
    response->connection = request->connection;
  }

  // char *response_buffer = (char *)malloc(BUFFER_SIZE * 2);
  char *content_encoding;
  bool is_text_file = (strstr(response->content_type, "text") != NULL) || (strstr(response->content_type, "application") != NULL);
  if (request->accept_encoding != NULL && is_text_file && file_to_send != NULL) {

    content_encoding = "Content-Encoding: gzip\r\n";
  }
  char *response_buffer = (char *)malloc(BUFFER_SIZE * 2);
  size_t response_header_length = snprintf(response_buffer, 2 * BUFFER_SIZE,
                                           "HTTP/1.1 %d %s\r\n"
                                           "Content-Type: %s\r\n"
                                           "Content-Length: %zu\r\n"
                                           "Connection: %s\r\n"
                                           "Keep-Alive: timeout=%d, max=%d\r\n"
                                           "%s"
                                           "Server: %s\r\n"
                                           "\r\n",
                                           response->status_code,
                                           http_message,
                                           response->content_type,
                                           response->content_length,
                                           response->connection,
                                           SERVER_TIMEOUT,
                                           MAX_REQUESTS_PER_CONNECTION,
                                           content_encoding,
                                           SERVER_NAME);

  if (file_to_send == NULL) {
    memcpy(response_buffer + response_header_length, response->body, strlen(response->body));
    send(client_fd, response_buffer, response_header_length + strlen(response->body), 0);
    free(response_buffer);
    return 0;
  }

  size_t total_data_sent = 0;
  int header_to_send = 1;

  do {
    size_t bytes_read;
    size_t chunk_to_send;
    if (header_to_send == 1) {
      bytes_read = fread(response_buffer + response_header_length, 1, 2 * BUFFER_SIZE - response_header_length, file_to_send);
      chunk_to_send = bytes_read + response_header_length;
    } else {
      bytes_read = fread(response_buffer, 1, 2 * BUFFER_SIZE, file_to_send);
      chunk_to_send = bytes_read;
    }

    size_t chunk_actually_sent = 0;
    while (chunk_actually_sent < chunk_to_send) {
      ssize_t bytes_sent = send(client_fd, response_buffer + chunk_actually_sent, chunk_to_send - chunk_actually_sent, 0);

      if (bytes_sent < 0) {
        printf("bytes_sent: %lu\n", bytes_sent);
        free(response_buffer);
        return -1;
      }
      chunk_actually_sent = chunk_actually_sent + bytes_sent;
    }
    total_data_sent = total_data_sent + chunk_actually_sent;
    header_to_send++;
    memset(response_buffer, 0, 2 * BUFFER_SIZE);
  } while (total_data_sent < response->content_length);

  free(response_buffer);
  fclose(file_to_send);
  return 0;
}
