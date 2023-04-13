#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 4)
    {
        printf("Sai tham so.\n");
        return 1;
    }

    int const destPort = atoi(argv[1]);
    char const *helloFN = argv[2];
    char const *logFN = argv[3];
    // printf("port:%d\nhello_file_name:%s\nlogFN:%s\n", destPort, helloFN, logFN );

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(destPort);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;        
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    printf("Listening on port %d\n", destPort);

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client = accept(listener, 
        (struct sockaddr *)&client_addr, 
        &client_addr_len
    );

    printf("Accepted socket %d from IP: %s:%d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port)
    );

    //Read and send greeting
    FILE *file;
    long file_size;

    file = fopen(helloFN, "r");
    if (file == NULL) {
        printf("fopen() failed\n");
        return 1;
    }

    // Determine the size of the file hello
    fseek(file, 0L, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    char greeting[file_size];    
    // Read the file into the greeting
    fread(greeting, file_size, 1, file);
    fclose(file);
    
    // send greeting
    send(client, greeting, file_size, 0);

    //reciece from client and log the content
    char buf[256];
    int ret;
    //changes are only written when file is closed
    while (1){
        //open file
        file = fopen(logFN, "a+");
        //receive from client
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret == 0) break;
        if (ret < sizeof(buf)) buf[ret] = 0;
        printf("%d bytes received\n", ret);

        //log to a file
        fprintf(file,"%s\n",buf);
        fclose(file);

    }
    close(client);
    close(listener);
    return 0;
}