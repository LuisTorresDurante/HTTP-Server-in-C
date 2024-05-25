#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_sockfd);
void send_response(int client_sockfd, const char *header, const char *content, const char *content_type);
void send_404(int client_sockfd);
void send_file(int client_sockfd, const char *filepath);

int main() {
    char buffer[BUFFER_SIZE];

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("webserver (socket)");
        return 1;
    }
    printf("socket created successfully\n");

    // Set the SO_REUSEADDR option
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("webserver (setsockopt)");
        close(sockfd);
        return 1;
    }

    // Create the address to bind the socket to
    struct sockaddr_in host_addr;
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) != 0) {
        perror("webserver (bind)");
        close(sockfd);
        return 1;
    }
    printf("socket successfully bound to address\n");

    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        close(sockfd);
        return 1;
    }
    printf("server listening for connections!\n");

    for (;;) {
        // Accept incoming connections
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
        if (newsockfd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        handle_client(newsockfd);
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}

void handle_client(int client_sockfd) {
    char buffer[BUFFER_SIZE];

    // Read from the socket
    int valread = read(client_sockfd, buffer, BUFFER_SIZE - 1);
    if (valread < 0) {
        perror("webserver (read)");
        return;
    }
    buffer[valread] = '\0';

    // Extract the requested file path
    char method[16], path[256], protocol[16];
    sscanf(buffer, "%s %s %s", method, path, protocol);

    // Only handle GET requests
    if (strcmp(method, "GET") != 0) {
        send_response(client_sockfd, "HTTP/1.0 501 Not Implemented\r\n", "Method not implemented", "text/plain");
        return;
    }

    // Remove leading slash from path
    if (path[0] == '/') memmove(path, path + 1, strlen(path));

    // Serve the requested file
    if (strlen(path) == 0) strcpy(path, "index.html");
    send_file(client_sockfd, path);
}

void send_response(int client_sockfd, const char *header, const char *content, const char *content_type) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "%sContent-Type: %s\r\nContent-Length: %zu\r\n\r\n%s", header, content_type, strlen(content), content); // Changed to include Content-Length header
    write(client_sockfd, response, strlen(response));
}


void send_404(int client_sockfd) {
    const char *header = "HTTP/1.0 404 Not Found\r\n";
    const char *content = "<html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_sockfd, header, content, "text/html");
}

void send_file(int client_sockfd, const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("webserver (open)");
        send_404(client_sockfd);
        return;
    }

    // Determine the file size
    off_t filesize = lseek(fd, 0, SEEK_END); // Changed to get file size
    lseek(fd, 0, SEEK_SET); // Reset file offset

    // Read the file content into the buffer
    char *filecontent = malloc(filesize + 1); // Changed to allocate memory based on file size
    if (filecontent == NULL) {
        perror("webserver (malloc)");
        close(fd);
        send_404(client_sockfd);
        return;
    }

    ssize_t bytes_read = read(fd, filecontent, filesize); // Changed to ensure full file is read
    if (bytes_read != filesize) {
        perror("webserver (read)");
        close(fd);
        free(filecontent);
        send_404(client_sockfd);
        return;
    }
    filecontent[filesize] = '\0'; // Null-terminate the file content
    close(fd);

    // Send the file content as HTTP response
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Length: %zd\r\n", filesize); // Changed to include Content-Length header
    send_response(client_sockfd, header, filecontent, "text/html"); // Changed to pass header and file content
    free(filecontent);
}
