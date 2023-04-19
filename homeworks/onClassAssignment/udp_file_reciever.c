#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Parse command line argument
    int port = atoi(argv[1]);

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket() failed");
        exit(1);
    }

    // Bind socket to local address and port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind() failed");
        exit(1);
    }

    printf("Listening on port %d\n", port);

    // Receive filename from client
    char filename[BUF_SIZE];
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    ssize_t filename_len = recvfrom(sockfd, filename, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (filename_len == -1) {
        perror("recvfrom() failed");
        exit(1);
    }
    filename[filename_len] = '\0';
    printf("Received filename: %s\n", filename);

    // Open file for writing
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("fopen() failed");
        exit(1);
    }

    // Receive file contents from client and write to file
    char buf[BUF_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len)) > 0) {
        if (fwrite(buf, 1, bytes_received, file) != bytes_received) {
            perror("fwrite() failed");
            exit(1);
        }
    }
    if (bytes_received == -1) {
        perror("recvfrom() failed");
        exit(1);
    }

    // Close file and socket
    fclose(file);
    close(sockfd);

    printf("File received successfully.\n");

    return 0;
}
