#include <Winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(SOCKET client_sockfd);
void send_response(SOCKET client_sockfd, const char *header, const char *content, const char *content_type);
void send_404(SOCKET client_sockfd);
void send_file(SOCKET client_sockfd, const char *filepath);

int main() {
    WSADATA wsaData;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in host_addr, client_addr;
    int client_addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Error creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    host_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) == SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Server listening for connections on port %d\n", PORT);

    for (;;) {
        newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
        if (newsockfd == INVALID_SOCKET) {
            printf("Accept failed with error: %d\n", WSAGetLastError());
            continue;
        }

        handle_client(newsockfd);

        closesocket(newsockfd);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}

void handle_client(SOCKET client_sockfd) {
    char buffer[BUFFER_SIZE];

    int valread = recv(client_sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (valread == SOCKET_ERROR) {
        printf("Recv failed with error: %d\n", WSAGetLastError());
        return;
    }
    buffer[valread] = '\0';

    char method[16], path[256], protocol[16];
    sscanf(buffer, "%s %s %s", method, path, protocol);

    if (strcmp(method, "GET") != 0) {
        send_response(client_sockfd, "HTTP/1.0 501 Not Implemented\r\n", "Method not implemented", "text/plain");
        return;
    }

    if (path[0] == '/') memmove(path, path + 1, strlen(path));

    if (strlen(path) == 0) strcpy(path, "index.html");
    send_file(client_sockfd, path);
}

void send_response(SOCKET client_sockfd, const char *header, const char *content, const char *content_type) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "%sContent-Type: %s\r\nContent-Length: %zu\r\n\r\n%s", header, content_type, strlen(content), content);
    send(client_sockfd, response, strlen(response), 0);
}

void send_404(SOCKET client_sockfd) {
    const char *header = "HTTP/1.0 404 Not Found\r\n";
    const char *content = "<html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_sockfd, header, content, "text/html");
}

void send_file(SOCKET client_sockfd, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("Error opening file");
        send_404(client_sockfd);
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *filecontent = malloc(filesize + 1);
    if (filecontent == NULL) {
        perror("Error allocating memory");
        fclose(file);
        send_404(client_sockfd);
        return;
    }

    fread(filecontent, 1, filesize, file);
    fclose(file);

    filecontent[filesize] = '\0';

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Length: %ld\r\n", filesize);
    send_response(client_sockfd, header, filecontent, "text/html");

    free(filecontent);
}
