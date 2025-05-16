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

void handle_request(int client_fd) {

  char *request_buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t bytes_received = recv(client_fd, request_buffer, BUFFER_SIZE, 0);

  if (bytes_received < 0) {
    perror("No bytes, mate!");
  }

  HttpRequest request;
  HttpResponse response;

  if (request_buffer != NULL) {
    printf("Request Headers\n");
    printf("------------------------------------------------\n");
    printf("%s", request_buffer);
  }

  parse_headers(request_buffer, &request, sizeof(request_buffer));

  if (prepare_response(&request, &response, client_fd)) {
    printf("Response sent sucessfully\n\n");
    printf("Connection with the client %d is closed\n", client_fd);
    printf("------------------------------------------------\n");
    printf("------------------------------------------------\n\n");
    printf("Listening for more clients....\n\n");
  } else {
    printf("Response failed to send");
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

  printf("Listening Socket Created brother!\n");

  // to reuse the port for recompiling purposes
  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serv_info;

  serv_info.sin_family = AF_INET;
  serv_info.sin_port = htons(PORT);
  serv_info.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (const struct sockaddr *)&serv_info, sizeof(serv_info)) <
      0) {
    perror("Error binding, mate!");
    return -1;
  }
  printf("Binding the socket to port and ip successful brother!\n");

  int connections_queue = 5;

  if (listen(sock_fd, connections_queue) < 0) {
    perror("Somehow cannot listen, mate!");
    return -1;
  }
  printf("listening on Port %d....\n", PORT);

  printf("Waiting for a client to connect...\n\n");

  // infinite loop for keep taking client connections
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    int client_socket = accept(sock_fd, NULL, NULL);

    if (client_socket < 0) {
      perror("Cannot establish connection with client, mate!");
      return -1;
    }

    printf("------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("A client is with socket number %d is connected\n\n", client_socket);

    if (getpeername(client_socket, (struct sockaddr *)&client_addr,
                    &client_addrlen) < 0) {
      printf("error: %s\n", strerror(errno));
    }

    char *clientaddrpresn = inet_ntoa(client_addr.sin_addr);

    printf("Client information\n");
    printf("------------------------------------------------\n");
    printf("Client Address Family: %d\n", client_addr.sin_family);
    printf("Client Port: %d\n", ntohs(client_addr.sin_port));
    printf("Client IP Address: %s\n\n", clientaddrpresn);

    handle_request(client_socket);
  }

  close(sock_fd);
  return 0;
}
