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

void handle_request(int client_fd, int client_id) {

  char *request_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t bytes_received = recv(client_fd, request_buffer, BUFFER_SIZE, 0);

  if (bytes_received < 0) {
    printf("\e[%sm[ERROR] [Client%d] No bytes received, sire!\e[0m\n",
           LOG_ERROR, client_id);
  }

  HttpRequest request;
  HttpResponse response;

  if (request_buffer != NULL) {
    // implement timeout bruh
    // printf("------------------------------------------------\n");
    // printf("\e[%sm[DEBUG] [Client:%d] Request Headers\e[0m\n", LOG_DEBUG,
    //        client_id);
    // printf("\e[%sm[DEBUG] [Client:%d] %s\e[0m\n", LOG_DEBUG, client_id,
    //        request_buffer);
  }

  parse_headers(request_buffer, &request, sizeof(request_buffer));

  if (prepare_response(&request, &response, client_fd, client_id)) {
    if (response.status_code != 200) {
      printf("\e[%sm[ERROR] [Client:%d] [%d] Response sent successfully\e[0m\n",
             LOG_ERROR, client_id, response.status_code);
    } else {
      printf(
          "\e[%sm[SUCCESS] [Client:%d] [%d] Response sent successfully\e[0m\n",
          LOG_SUCCESS, client_id, response.status_code);
    }
    // printf("------------------------------------------------\n");
    printf(
        "\e[%sm[USER] [Client:%d] Connection with the Client is closed\e[0m\n",
        LOG_USER_INFO, client_id);
    // printf("------------------------------------------------\n");
    printf("\e[%sm[INFO] Listening for more clients...\e[0m\n", LOG_INFO);
  } else {
    printf("\e[%sm[ERROR] [Client%d] Response failed to send, sire! Closing "
           "Connection Anyway\e[0m\n",
           LOG_ERROR, client_id);
  }
  free(request_buffer);
  close(client_fd);
}

int main() {

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd < 0) {
    printf("%d\n", sock_fd);
    perror("Error creating socket, mate!");
    return -1;
  }

  printf("\e[%sm[INFO] Listening Socket Created, sire!\e[0m\n", LOG_INFO);

  // to reuse the port for recompiling purposes
  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serv_info;

  serv_info.sin_family = AF_INET;
  serv_info.sin_port = htons(PORT);
  serv_info.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (const struct sockaddr *)&serv_info, sizeof(serv_info)) <
      0) {
    perror("Error binding, sire!");
    return -1;
  }
  printf("\e[%sm[INFO] Binding port and ip successful "
         "sire!\e[0m\n",
         LOG_INFO);

  int connections_queue = 5;

  if (listen(sock_fd, connections_queue) < 0) {
    perror("Somehow cannot listen, mate!");
    return -1;
  }

  printf("\e[%sm[INFO] Server Listening on Port %d....\e[0m\n", LOG_INFO, PORT);
  printf("\e[%sm[INFO] Waiting for a client to connect...\e[0m\n", LOG_INFO);

  // infinite loop for keep taking client connections
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    int client_socket = accept(sock_fd, NULL, NULL);

    if (client_socket < 0) {
      printf(
          "\e[%sm[ERROR] Cannot establish connection with client, mate!\e[0m\n",
          LOG_ERROR);
      return -1;
    }

    if (getpeername(client_socket, (struct sockaddr *)&client_addr,
                    &client_addrlen) < 0) {
      printf("error: %s\n", strerror(errno));
    }

    // char *clientaddrpresn = inet_ntoa(client_addr.sin_addr);
    // printf("------------------------------------------------\n");
    printf("\e[%sm[SUCCESS] [Client:%d] A Client is connected\e[0m\n",
           LOG_SUCCESS, ntohs(client_addr.sin_port));
    // printf("------------------------------------------------\n");

    handle_request(client_socket, ntohs(client_addr.sin_port));
  }

  close(sock_fd);
  return 0;
}
