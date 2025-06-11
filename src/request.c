#include "../include/request.h"
#include "../include/common.h"
#include "../include/queue-ds.h"
#include "../include/response.h"
#include "../include/utils.h"
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool match_and_set_header(char *header_key, char *header_value,
                          HttpRequest *request) {

  // convert all the characters of header-key to lowerbcase due to case
  // insensitivity
  for (int i = 0; header_key[i] != '\0'; i++) {
    header_key[i] = tolower(header_key[i]);
  }

  if (strcmp(header_key, "host") == 0) {

    request->host = header_value;
    return true;
  } else if (strcmp(header_key, "content-length") == 0) {

    request->content_length = atoi(header_value);
    return true;
  } else if (strcmp(header_key, "content-type") == 0) {

    request->content_type = header_value;
    return true;
  } else {

    return false;
  }
}

int parse_request(char *request_buffer, HttpRequest *request,
                  HttpResponse *response, int client_fd) {

  // the header key-value pair starts here minus the request line
  char *header_start_pointer = strstr(request_buffer, "\r\n");
  if (header_start_pointer == NULL) {
    goto bad_request;
  }
  header_start_pointer++;

  char *header_end_pointer = strstr(request_buffer, "\r\n\r\n");
  if (header_end_pointer == NULL) {
    goto bad_request;
  }
  header_end_pointer += 3;

  // parse request line
  char general_delimiter = ' ';
  char *start_of_path = strchr(request_buffer, general_delimiter);
  if (start_of_path == NULL) {
    goto bad_request;
  }
  start_of_path++;

  char *end_of_path = strchr(start_of_path, general_delimiter);
  if (end_of_path == NULL) {
    goto bad_request;
  }
  *end_of_path = '\0';

  char *end_of_method = strchr(request_buffer, general_delimiter);
  if (end_of_method == NULL) {
    goto bad_request;
  }
  *end_of_method = '\0';

  request->method = request_buffer;
  request->path = start_of_path;

  char *token = NULL;

  while ((token = strtok_r(header_start_pointer, "\r\n",
                           &header_start_pointer)) != NULL) {

    if (token == header_end_pointer + 1) {
      request->body = token;
      break;
    }

    if (token == NULL) {
      goto bad_request;
    }

    char *terminate_key = strchr(token, ':');
    if (terminate_key == NULL) {
      goto bad_request;
    }
    *terminate_key = '\0';

    char *value_start = terminate_key + 1;
    while (*value_start == ' ') {
      value_start++;
    }
    match_and_set_header(token, value_start, request);
  }

  if (strcmp(request->method, "POST") == 0 &&
      strcmp(request->path, "/contact") == 0) {
    if (request->content_length == 0) {
      printf("content-length not given\n");
      goto bad_request;
    }
    if (request->content_type == NULL ||
        strcmp(request->content_type, "application/x-www-form-urlencoded") !=
            0) {
      printf("content-type not given\n");
      goto bad_request;
    }

    response->status_code = 201;
    if (strlen(request->body) < request->content_length) {
      printf("request body is incomplete\n");

      int bytes_received =
          recv(client_fd, &request->body[strlen(request->body) - 1], 5, 0);
      if (bytes_received <= 0) {
        goto bad_request;
      }
    }
  }

  return 0;

bad_request:
  response->status_code = 400;
  return -1;
}

void *handle_request(Node *p_client) {

  int client_fd = p_client->client_fd;
  int client_id = p_client->client_id;
  free(p_client);
  pthread_t self_id = pthread_self();
  log_to_console(&logs.info, "Thread:%d is handling this client", self_id,
                 client_id);

  // timout logic
  struct timeval timeout;
  timeout.tv_sec = 5;  // Timeout in seconds
  timeout.tv_usec = 0; // Timeout in microseconds

  if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    perror("setsockopt failed");
    // Handle error
  }

  char *request_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t bytes_received = 0;
  bytes_received = recv(client_fd, request_buffer, BUFFER_SIZE, 0);

  if (bytes_received <= 0) {
    log_to_console(&logs.user, "Client is Inactive", 0, client_id);
    goto close_connection;
  }

  HttpRequest request;
  HttpResponse response;
  // zeroing out the all the values of the initialized struct so that i don't
  // accidentally use a garbage value or memory address
  memset(&request, 0, sizeof request);
  memset(&response, 0, sizeof response);

  if (parse_request(request_buffer, &request, &response, client_fd) != 0) {
    response.status_code = 400;
  }

  if (send_response(&request, &response, client_fd, client_id) == 0) {
    if (response.status_code != 200 && response.status_code != 201) {
      log_to_console(&logs.error, "[%d] Response sent with Error",
                     response.status_code, client_id);
    } else {
      log_to_console(&logs.success, "[%d] Response sent successfully",
                     response.status_code, client_id);
    }
  } else {
    log_to_console(&logs.error, "Response failed to send, sire!", 0, client_id);
  }

close_connection:
  printf("------------------------------------------------\n");
  log_to_console(&logs.user, "Connection with the Client is closed", 0,
                 client_id);
  printf("------------------------------------------------\n");
  free(request_buffer);
  close(client_fd);
  return NULL;
}
