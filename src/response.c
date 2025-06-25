#include "../include/response.h"
#include "../include/request.h"
#include "../include/utils.h"

#include <dirent.h>
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
  case 404:
    return http_status_messages[4];
    break;
  case 415:
    return http_status_messages[5];
    break;
  case 500:
    return http_status_messages[6];
    break;
  default:
    return http_status_messages[0];
  }
}

FILE *open_static_file(HttpRequest *request, HttpResponse *response,
                       int client_id) {
  extern char *public_directory;

  char file_path[30];

  if (strcmp(request->path, "/home") == 0 || strcmp(request->path, "/") == 0) {
    snprintf(file_path, 30, "%s%s", public_directory, "/index.html");

  } else if (strchr(request->path, '.') == NULL) {

    snprintf(file_path, 30, "%s%s.html", public_directory, request->path);

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

    snprintf(file_path, 30, "%s%s", public_directory, request->path);
  }

  log_to_debug(&logs.debug, "Actual Resource Path found: %s", file_path,
               client_id);
  FILE *file_to_read = fopen(file_path, "rb");

  if (file_to_read == NULL) {
    log_to_console(&logs.error, "The File doesn't exist, sire!", 0, client_id);
    response->status_code = 404;
    return NULL;
  }

  log_to_console(&logs.user, "Requested Static file opened, sire!", 0,
                 client_id);
  struct stat file_statbuf;
  int stat_result = fstat(fileno(file_to_read), &file_statbuf);

  if (stat_result < 0) {
    log_to_console(&logs.error,
                   "Failed to get the file information of the file", 0,
                   client_id);
    fclose(file_to_read);
    response->status_code = 500;
    return NULL;
  }

  // size_t required_file_size = file_statbuf.st_size;
  // remember to free this
  // char *response_buffer = (char *)malloc(required_file_size);

  // size_t bytes_read = fread(response_buffer, 1, required_file_size, file_to_read);

  // printf("bytes_read: %lu\n", bytes_read);
  // if (bytes_read != required_file_size) {
  //   log_to_console(&logs.error, "Read Operation failed, sire!", 0, client_id);
  //   fclose(file_to_read);
  //   response->status_code = 500;
  //   return NULL;
  // }
  // response->body = response_buffer;
  // response->body[required_file_size - 1] = '\0';
  // log_to_console(&logs.user, "File Read is successful, sire!", 0, client_id);
  // fclose(file_to_read);
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

  if (response->status_code != 200) {
    response->body = (char *)malloc(BUFFER_SIZE);
    snprintf(response->body, BUFFER_SIZE,
             "<html><body><h1>%d, %s</h1></body></html>", response->status_code,
             http_message);
    response->content_length = strlen(response->body);
  }

  if (request->connection != NULL &&
      strcmp("close", request->connection) == 0) {
    response->connection = request->connection;
  }

  char *response_header = (char *)malloc(BUFFER_SIZE);
  size_t response_header_length = snprintf(response_header, BUFFER_SIZE,
                                           "HTTP/1.1 %d %s\r\n"
                                           "Content-Type: %s\r\n"
                                           "Content-Length: %zu\r\n"
                                           "Connection: %s\r\n"
                                           "Keep-Alive: timeout=%d, max=%d\r\n"
                                           "Server: %s\r\n"
                                           "\r\n",
                                           response->status_code,
                                           http_message,
                                           response->content_type,
                                           response->content_length,
                                           response->connection,
                                           SERVER_TIMEOUT,
                                           MAX_REQUESTS_PER_CONNECTION,
                                           SERVER_NAME);

  ssize_t header_bytes_sent = send(client_fd, response_header, response_header_length, 0);
  if (header_bytes_sent < 0) {
    free(response_header);
    return -1;
  }

  free(response_header);

  if (file_to_send == NULL) {
    send(client_fd, response->body, strlen(response->body), 0);
    return 0;
  }

  response->body = malloc(2 * BUFFER_SIZE);
  size_t total_data_sent = 0;
  int iteration = 0;

  do {
    printf("iteration: %d\n", iteration++);
    memset(response->body, 0, 2 * BUFFER_SIZE);

    size_t bytes_read = fread(response->body, 1, 2 * BUFFER_SIZE, file_to_send);

    size_t chunk_to_send = bytes_read;
    size_t chunk_actually_sent = 0;
    while (chunk_actually_sent < chunk_to_send) {
      ssize_t bytes_sent = send(client_fd, response->body + chunk_actually_sent, chunk_to_send - chunk_actually_sent, 0);
      if (bytes_sent < 0) {
        free(response->body);
        return -1;
      }
      chunk_actually_sent = chunk_actually_sent + bytes_sent;
    }
    total_data_sent = total_data_sent + chunk_actually_sent;
  } while (total_data_sent < response->content_length);
  free(response->body);
  // memcpy(response_buffer + response_header_length, response->body,
  //        response->content_length);

  // size_t bytes_read = fread(response_buffer + response_header_length, 1, BUFFER_SIZE, file_to_send);

  // printf("bytes_read: %lu\n", bytes_read);
  // if (bytes_read <= 0) {
  //   log_to_console(&logs.error, "Read Operation failed, sire!", 0, client_id);
  //   fclose(file_to_send);
  //   response->status_code = 500;
  //   return NULL;
  // }
  // log_to_console(&logs.user, "File Read is successful, sire!", 0, client_id);
  // free(response->body);

  // // make sure all the bytes get sent to the client even in chunks if necessary
  // size_t total_to_send = response_header_length + response->content_length;
  // size_t total_sent = 0;
  // while (total_sent < total_to_send) {
  //   ssize_t bytes_sent = send(client_fd, response_buffer + total_sent, total_to_send - total_sent, 0);
  //   if (bytes_sent < 0) {
  //     free(response_buffer);
  //     return -1;
  //   }
  //   total_sent = total_sent + bytes_sent;
  // }

  // free(response_buffer);
  if (file_to_send != NULL) {
    fclose(file_to_send);
  }
  return 0;
}
