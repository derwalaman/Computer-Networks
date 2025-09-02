// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <asm-generic/socket.h>

#define PORT 8080

void send_response(int client_socket) {
    FILE *html_file = fopen("index.html", "r");
    if (html_file == NULL) {
        perror("Could not open index.html");
        return;
    }

    // HTTP Response Header
    char response_header[] = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n";

    // Send Header
    send(client_socket, response_header, strlen(response_header), 0);

    // Send File Content
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), html_file)) {
        send(client_socket, buffer, strlen(buffer), 0);
    }

    fclose(html_file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Attach socket to port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening on http://localhost:%d\n", PORT);

    // 5. Accept connections
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }

        // Optional: Read and print HTTP request
        char request[3000];
        read(client_socket, request, sizeof(request));
        printf("Received Request:\n%s\n", request);

        // 6. Send HTML response
        send_response(client_socket);

        // 7. Close client socket
        close(client_socket);
    }

    return 0;
}