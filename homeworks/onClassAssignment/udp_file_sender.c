#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <filename> <ip_address> <port>\n", argv[0]);
        exit(1);
    }

    // Parse command line arguments
    char *filename = argv[1];
    char *ip_address_str = argv[2];
    int port = atoi(argv[3]);

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket() failed");
        exit(1);
    }

    // Prepare server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address_str, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_address_str);
        exit(1);
    }

    // Open file for reading
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen() failed");
        exit(1);
    }

    // Send filename to server
    size_t filename_len = strlen(filename);
    if (sendto(sockfd, filename, filename_len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto() failed");
        exit(1);
    }

    // Send file contents to server
    char buf[BUF_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buf, 1, BUF_SIZE, file)) > 0) {
        if (sendto(sockfd, buf, bytes_read, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            perror("sendto() failed");
            exit(1);
        }
    }

    // Close file and socket
    fclose(file);
    close(sockfd);

    return 0;
}
