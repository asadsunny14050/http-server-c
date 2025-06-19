#include <arpa/inet.h> // Include for inet_addr and other functions
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h> // Include for sockaddr_in structure
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Include POSIX socket library for Linux
#include <sys/time.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/queue-ds.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/utils.h"

// pool of threads
pthread_t THREAD_POOL[THREAD_POOL_SIZE];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

// keeps track of all the accepted connections from different clients,
// waiting to be handled by one of the threads
Queue WORK_QUEUE = {0};

void *single_thread_lifetime() {

  while (1) {

    Node *p_client;

    pthread_mutex_lock(&lock);

    p_client = dequeue(&WORK_QUEUE);
    if (p_client == NULL) {
      pthread_cond_wait(&condition_var, &lock);
      p_client = dequeue(&WORK_QUEUE);
    }

    pthread_mutex_unlock(&lock);

    if (p_client != NULL) {
      handle_request(p_client);
    }
  }
}

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

  log_to_console(&logs.info,
                 "Provisioning %d threads for the workload as commanded, sire!",
                 THREAD_POOL_SIZE, 0);
  printf("[Spinning Threads]: ");
  for (int i = 0; i < THREAD_POOL_SIZE; i++) {

    usleep(200000);

    if (i == THREAD_POOL_SIZE - 1) {

      printf("#\n");
    } else {
      printf("#");
    }

    fflush(stdout);
    usleep(500);
    pthread_create(&THREAD_POOL[i], NULL, single_thread_lifetime, NULL);
  }

  log_to_console(&logs.info, "Server Listening on Port %d.... ", PORT, 0);
  log_to_console(&logs.info, "Waiting for a client to connect.... ", 0, 0);

  return sock_fd;
}

char *public_directory = NULL;

int main(int argc, char **argv) {

  if (argc != 2) {
    perror("Usage: cpider-web-server <directory>");
    exit(1);
  }
  public_directory = argv[1];

  DIR *dir = opendir(public_directory);
  if (!dir) {
    perror("cannot find path, input a directory that actually exists lol!");
    exit(1);
  }

  log_to_console(&logs.success, "Scanning Directory, sire!", 0, 0);
  log_to_console(&logs.success, "Scanning Assets for Service, sire!", 0, 0);

  struct dirent *dir_entry;
  bool index_html_exists = false;

  for (int i = 0; (dir_entry = readdir(dir)) != NULL; i++) {

    if ((i = 0 && dir_entry == NULL)) {

      perror(
          "directory's empty, what do you want me to serve, nothingburger?\n");
      exit(1);
    }

    if (strcmp(dir_entry->d_name, "index.html") == 0)
      index_html_exists = true;

    if (!strcmp(dir_entry->d_name, ".") || dir_entry->d_name[0] == '.' ||
        !strcmp(dir_entry->d_name, "..")) {
      continue;
    }

    char *file_extension = strchr(dir_entry->d_name, '.');
    if (file_extension == NULL) {
      continue;
    }

    if (validate_content_type(file_extension + 1) != 0) {
      log_to_debug(&logs.warning, "File: \e[35m%s\e[0m is not supported",
                   dir_entry->d_name, 0);
    }
  }

  if (!index_html_exists) {

    log_to_debug(
        &logs.warning,
        "File: \e[35mindex.html\e[0m for default home page is not given", NULL,
        0);
  }

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
      continue;
    }

    if (getpeername(client_socket, (struct sockaddr *)&client_addr,
                    &client_addrlen) < 0) {
      printf("error: %s\n", strerror(errno));
    }

    printf("------------------------------------------------\n");
    log_to_console(&logs.success, "A Client is connected", 0,
                   ntohs(client_addr.sin_port));

    printf("------------------------------------------------\n");
    pthread_mutex_lock(&lock);
    if (enqueue(&WORK_QUEUE, client_socket, ntohs(client_addr.sin_port)) < 0) {

      printf("failed to add to the queue, sire!\n");
    };
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&lock);
  }

  close(sock_fd);
  return 0;
}
