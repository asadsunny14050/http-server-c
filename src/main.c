#include <arpa/inet.h> // Include for inet_addr and other functions
#include <errno.h>
#include <netinet/in.h> // Include for sockaddr_in structure
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Include POSIX socket library for Linux
#include <unistd.h>

#include "../include/common.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/utils.h"

void handle_request(int client_fd, int client_id) {

  char *request_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t bytes_received = recv(client_fd, request_buffer, BUFFER_SIZE, 0);

  if (bytes_received < 0) {
    log_to_console(&logs.error, "No bytes received, sire!", 0, client_id);
  }

  if (request_buffer != NULL) {
    // implement timeout bruh
    // printf("------------------------------------------------\n");
    // printf("\e[%sm[DEBUG] [Client:%d] Request Headers\e[0m\n", LOG_DEBUG,
    //        client_id);
    // printf("\e[%sm[DEBUG] [Client:%d] %s\e[0m\n", LOG_DEBUG, client_id,
    //        request_buffer);
  }

  HttpRequest request;
  HttpResponse response;

  parse_headers(request_buffer, &request, sizeof(request_buffer));

  if (prepare_response(&request, &response, client_fd, client_id)) {
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
  log_to_console(&logs.info, "Listening for more clients...", 0, 0);
}

int main() {

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd < 0) {
    log_to_console(&logs.error, "Listening Socket Created, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Listening Socket Created, sire!", 0, 0);

  // to reuse the port for recompiling purposes
  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serv_info;

  serv_info.sin_family = AF_INET;
  serv_info.sin_port = htons(PORT);
  serv_info.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (const struct sockaddr *)&serv_info, sizeof(serv_info)) <
      0) {

    log_to_console(&logs.error, "Error binding, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Binding port and ip successful, sire!", 0, 0);
  int connections_queue = 5;

  if (listen(sock_fd, connections_queue) < 0) {
    log_to_console(&logs.error, "Cannot listen somehow, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Server Listening on Port %d.... ", PORT, 0);
  log_to_console(&logs.info, "Waiting for a client to connect.... ", 0, 0);

  // infinite loop for keep taking client connections
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    int client_socket = accept(sock_fd, 0, 0);

    if (client_socket < 0) {
      log_to_console(&logs.error,
                     "Cannot establish connection with client, sire!", 0, 0);
      return -1;
    }

    if (getpeername(client_socket, (struct sockaddr *)&client_addr,
                    &client_addrlen) < 0) {
      printf("error: %s\n", strerror(errno));
    }

    printf("------------------------------------------------\n");
    log_to_console(&logs.success, "A Client is connected", 0,
                   ntohs(client_addr.sin_port));

    printf("calling handle_request\n");
    handle_request(client_socket, ntohs(client_addr.sin_port));
  }

  close(sock_fd);
  return 0;
}
