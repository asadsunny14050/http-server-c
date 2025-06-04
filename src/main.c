#include <arpa/inet.h> // Include for inet_addr and other functions
#include <errno.h>
#include <netinet/in.h> // Include for sockaddr_in structure
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Include POSIX socket library for Linux
#include <unistd.h>

#include "../include/common.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/utils.h"

pthread_t THREAD_POOL[THREAD_POOL_SIZE];

int init_server(struct sockaddr_in *serv_info) {

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd < 0) {
    log_to_console(&logs.error, "Listening Socket Created, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Listening Socket Created, sire!", 0, 0);

  // to reuse the port for recompiling purposes
  int opt = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  serv_info->sin_family = AF_INET;
  serv_info->sin_port = htons(PORT);
  serv_info->sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (const struct sockaddr *)serv_info, sizeof(*serv_info)) <
      0) {

    log_to_console(&logs.error, "Error binding, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Binding port and ip successful, sire!", 0, 0);
  int connections_queue = 256;

  if (listen(sock_fd, connections_queue) < 0) {
    log_to_console(&logs.error, "Cannot listen somehow, sire!", 0, 0);
    return -1;
  }

  log_to_console(&logs.info, "Server Listening on Port %d.... ", PORT, 0);
  log_to_console(&logs.info, "Waiting for a client to connect.... ", 0, 0);

  return sock_fd;
}

int main() {

  struct sockaddr_in serv_info;

  int sock_fd = init_server(&serv_info);
  if (sock_fd < 0) {

    log_to_console(&logs.error, "Error initializing server, sire!", 0, 0);
    exit(1);
  }

  // infinite loop for keep taking client connections
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    int client_socket =
        accept(sock_fd, (struct sockaddr *)&client_addr, &client_addrlen);

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

    pthread_t thread;
    int *p_client = malloc(2 * sizeof(int));
    p_client[0] = client_socket;
    p_client[1] = ntohs(client_addr.sin_port);

    pthread_create(&thread, NULL, handle_request, p_client);

    log_to_console(&logs.info, "Listening for more clients...", 0, 0);
  }

  close(sock_fd);
  return 0;
}
