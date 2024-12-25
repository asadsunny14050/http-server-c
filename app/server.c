#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

int main()
{
    WSADATA wsa_data;
    int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    if (wsa_result != 0)
    {
        perror("WSA startup failed for some stupid fucking reason");
        return -1;
    }

    printf("Kick started WSA, Yay!!!!\n\n");

    struct sockaddr_in serv_info;
    struct sockaddr_in client_addr;

    SOCKET sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
    {
        printf("%d\n", sock_fd);
        perror("Error creating socket, mate!");
        return -1;
    }

    printf("Socket Created brother!\n\n");
  

    serv_info.sin_family = AF_INET;
    serv_info.sin_port = htons(3001);
    serv_info.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (const struct sockaddr *) &serv_info, sizeof(serv_info) ) < 0) {
        perror("Error binding, mate!");
        return -1;
    }
    printf("Bind successful brother!\n\n");

    int connections_queue = 5;

    if (listen(sock_fd, connections_queue) < 0) {
        perror("Somehow cannot listen, mate!");
        return -1;
    }
    printf("Bind successful brother!\n\n");
    printf("Waiting for a client to connect\n\n");

    int client_addrlen = sizeof(client_addr);

    SOCKET new_socket = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addrlen);

    if (new_socket < 0) {
        perror("Cannot establish connection with client, mate!");
        return -1;
    }

    closesocket(sock_fd);
    WSACleanup();
    return 0;
}
