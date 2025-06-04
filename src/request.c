#include "../include/request.h"
#include "../include/common.h"
#include "../include/response.h"
#include "../include/utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void parse_headers(char *request_buffer, HttpRequest *request,
                   int buffer_length) {

  // the header key-value pair starts here minus the request line
  char *header_start_pointer = strstr(request_buffer, "\r\n");
  // parse request line
  char general_delimiter = ' ';
  char *start_of_path = strchr(request_buffer, general_delimiter) + 1;
  char *end_of_path = strchr(start_of_path, general_delimiter);
  *end_of_path = '\0';

  char *end_of_method = strchr(request_buffer, general_delimiter);
  *end_of_method = '\0';

  request->method = request_buffer;
  request->path = start_of_path;
  // printf("Successfully Parsed Headers:\n");
  // printf("method:%s\n", request->method);
  // printf("path:%s\n", request->path);
  // printf("header:\n%s\n", header_start_pointer);
  // printf("------------------------------------------------\n");
}

void *handle_request(void *p_client) {

  int *client = (int *)p_client;
  int client_fd = client[0];
  int client_id = client[1];
  free(p_client);
  pthread_t self_id = pthread_self();
  log_to_console(&logs.info, "Handling Client's Request with Thread:%d",
                 self_id, client_id);

  char *request_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
  sleep(5);

  ssize_t bytes_received = 0;
  bytes_received = recv(client_fd, request_buffer, BUFFER_SIZE, 0);
  while (bytes_received == 0) {

    if (bytes_received > 0) {
      log_to_console(&logs.debug, "Bytes received from client: %d",
                     bytes_received, client_id);
      break;
    }
  }

  if (request_buffer != NULL) {
    // implement timeout bruh
  }

  HttpRequest request;
  HttpResponse response;

  parse_headers(request_buffer, &request, sizeof(request_buffer));

  if (prepare_response(&request, &response, client_fd, client_id) == 0) {
    if (response.status_code != 200) {
      log_to_console(&logs.error, "[%d] Response sent successfully",
                     response.status_code, client_id);
    } else {
      log_to_console(&logs.success, "[%d] Response sent successfully",
                     response.status_code, client_id);
    }
    log_to_console(&logs.user, "Connection with the Client is closed", 0,
                   client_id);
  } else {
    log_to_console(&logs.error,
                   "Response failed to send, sire! Closing Connection Anyway",
                   0, client_id);
  }
  printf("------------------------------------------------\n");
  free(request_buffer);
  close(client_fd);
  return NULL;
}
