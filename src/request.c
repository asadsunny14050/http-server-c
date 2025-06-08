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

bool match_header(char *header_key, char *header_value, HttpRequest *request) {

  // convert all the characters of header-key to lowerbcase due to case
  // insensitivity
  for (int i = 0; header_key[i] != '\0'; i++) {
    header_key[i] = tolower(header_key[i]);
  }

  if (strcmp(header_key, "host") == 0) {

    printf("Value: %s\n", header_value);
    request->host = header_value;
    return true;

  } else if (strcmp(header_key, "content-length") == 0) {

    printf("Value: %s\n", header_value);
    request->content_length = atoi(header_value);

    return true;
  } else if (strcmp(header_key, "content-type") == 0) {

    printf("Value: %s\n", header_value);
    request->content_type = header_value;

    return true;
  } else {

    printf("Unwanted Header key.\n");
    return false;
  }
}

int parse_request(char *request_buffer, HttpRequest *request,
                  HttpResponse *response) {

  printf("%s", request_buffer);
  int buffer_length = strlen(request_buffer);
  for (int i = 0; i < buffer_length; i++) {
    if (request_buffer[i] == '\r') {
      printf("\\r");
    } else if (request_buffer[i] == '\n') {
      printf("\\n\n");
    } else {

      printf("%c", request_buffer[i]);
    }
  }
  printf("\n");

  // the header key-value pair starts here minus the request line
  char *header_start_pointer = strstr(request_buffer, "\r\n") + 1;
  char *header_end_pointer = strstr(request_buffer, "\r\n\r\n") + 1;
  if (header_end_pointer == NULL) {
    // return bad request;
    goto early_return_routine;
  }

  // parse request line
  char general_delimiter = ' ';
  char *start_of_path = strchr(request_buffer, general_delimiter);
  if (start_of_path == NULL) {
    goto early_return_routine;
  }
  start_of_path++;

  char *end_of_path = strchr(start_of_path, general_delimiter);
  if (end_of_path == NULL) {
    goto early_return_routine;
  }
  *end_of_path = '\0';

  char *end_of_method = strchr(request_buffer, general_delimiter);
  if (end_of_method == NULL) {
    goto early_return_routine;
  }
  *end_of_method = '\0';

  request->method = request_buffer;
  request->path = start_of_path;

  // parse headers
  printf("the tokens: \n");
  int token_line = 1;
  char *token = strtok(header_start_pointer, "\r\n");
  while (token != NULL) {

    char *terminate_key = strchr(token, ':');
    if (terminate_key == NULL) {
      goto early_return_routine;
    }
    *terminate_key = '\0';

    char *value_start = terminate_key + 1;
    while (*value_start == ' ') {
      value_start++;
    }
    printf("key: %s, value: %s\n", token, value_start);
    match_header(token, value_start, request);

    token = strtok(NULL, "\r\n");
    token_line++;
  }

  printf("parsing header finished: displaying request object\n");
  printf("method: %s\n", request->method);
  printf("path: %s\n", request->path);
  printf("host: %s\n", request->host);
  printf("content_type: %s\n", request->content_type);
  printf("content_length: %d\n", request->content_length);

  return 0;

early_return_routine:
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

  if (parse_request(request_buffer, &request, &response) != 0) {
    response.status_code = 400;
    log_to_console(&logs.error, "[%d] Response sent successfully",
                   response.status_code, client_id);
    goto close_connection;
  }

  if (prepare_response(&request, &response, client_fd, client_id) == 0) {
    if (response.status_code != 200) {
      log_to_console(&logs.error, "[%d] Response sent successfully",
                     response.status_code, client_id);
    } else {
      log_to_console(&logs.success, "[%d] Response sent successfully",
                     response.status_code, client_id);
    }
  } else {
    log_to_console(&logs.error, "Response failed to send, sire!", 0, client_id);
  }
close_connection:
  log_to_console(&logs.user, "Connection with the Client is closed", 0,
                 client_id);
  printf("------------------------------------------------\n");
  free(request_buffer);
  close(client_fd);
  return NULL;
}
