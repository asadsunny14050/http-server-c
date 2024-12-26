#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #ifdef _WIN32                      // Check if compiling for Windows
// #include <winsock2.h>              // Include Winsock library for Windows
// #pragma comment(lib, "ws2_32.lib") // Link the Winsock library
// #else                              // If not Windows, assume Linux (POSIX)
#include <sys/socket.h> // Include POSIX socket library for Linux
#include <netinet/in.h> // Include for sockaddr_in structure
#include <arpa/inet.h>  // Include for inet_addr and other functions
#include <unistd.h>     // For close() function on Linux
// #endif

#define PORT 3001

int main()
{
    // if (_WIN32) {

    // WSADATA wsa_data;
    // int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // if (wsa_result != 0)
    // {
    //     perror("WSA startup failed for some stupid fucking reason");
    //     return -1;
    // }

    // printf("Kick started WSA, Yay!!!!\n\n");

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

    int new_socket = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addrlen);

    if (new_socket < 0)
    {
        perror("Cannot establish connection with client, mate!");
        return -1;
    }

    printf("A client is connected\nHis socket number is %d\n\n", new_socket);

    const char *msg = "Hello World!";

    if (send(sock_fd, msg, sizeof(msg), 0) < 0)
    {
        perror("Failed to send message");
        return -1;
    }

    // close(sock_fd);

    return 0;
}
