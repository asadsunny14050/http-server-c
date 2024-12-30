#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
// #ifdef _WIN32                      // Check if compiling for Windows
// #include <winsock2.h> // Include Winsock library for Windows
// #pragma comment(lib, "ws2_32.lib") // Link the Winsock library
// #else                              // If not Windows, assume Linux (POSIX)
#include <sys/socket.h> // Include POSIX socket library for Linux
#include <netinet/in.h> // Include for sockaddr_in structure
#include <arpa/inet.h>  // Include for inet_addr and other functions
#include <unistd.h>
#include <errno.h>
// For close() function on Linux
// #endif

#define PORT 4221
#define BUFFER_SIZE 3000

void handle_request(int client_fd)
{

    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);

    if (bytes_received < 0)
    {
        perror("No bytes, mate!");
    }

    char *message = "HTTP/1.1 200 OK\r\n\r\n";

    if (send(client_fd, message, strlen(message), 0) > 0)
    {
        printf("Response sent sucessfully\n");
    }
    else
    {
        printf("Response not sent");
    }
    free(buffer);
    // closesocket(client_fd);
}

int main()
{
    // if (_WIN32)
    // {

    //     WSADATA wsa_data;
    //     int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    //     if (wsa_result != 0)
    //     {
    //         perror("WSA startup failed for some stupid fucking reason");
    //         return -1;
    //     }

    //     printf("Kick started WSA, Yay!!!!\n\n");
    // }

    struct sockaddr_in serv_info;
    struct sockaddr_in client_addr;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
    {
        printf("%d\n", sock_fd);
        perror("Error creating socket, mate!");
        return -1;
    }

    printf("Socket Created brother!\n\n");

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        printf("SO_REUSEADDR failed: %s \n", strerror(errno));
        return 1;
    }

    serv_info.sin_family = AF_INET;
    serv_info.sin_port = htons(PORT);
    serv_info.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (const struct sockaddr *)&serv_info, sizeof(serv_info)) < 0)
    {
        perror("Error binding, mate!");
        return -1;
    }
    printf("Bind successful brother!\n\n");

    int connections_queue = 5;

    if (listen(sock_fd, connections_queue) < 0)
    {
        perror("Somehow cannot listen, mate!");
        return -1;
    }
    printf("listening on Port %d....\n\n", PORT);

    printf("Waiting for a client to connect...\n\n");

    int client_addrlen = sizeof(client_addr);

    // infinite loop for keep taking client connections

    while (1)
    {

        int new_socket = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addrlen);

        if (new_socket < 0)
        {
            perror("Cannot establish connection with client, mate!");
            return -1;
        }

        printf("new socket is %d\n\n", new_socket);

        printf("A client is connected\nHis socket number is %d\n\n", new_socket);

        handle_request(new_socket);
    }

    // close(sock_fd);

    return 0;
}
