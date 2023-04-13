#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Sai tham so.\n");
        return 1;
    }

    char const *destIP = argv[1];
    int const destPort = atoi(argv[2]);

    // printf("ip: %s\nport:%d\n", destIP, destPort);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(destIP);
    addr.sin_port = htons(destPort);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[256];

    //Receive greeting from server
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret < sizeof(buf)) buf[ret] = 0;
    printf("%d bytes received: %s\n", ret, buf);
    // char *msg = "Hello server\n";
    // send(client, msg, strlen(msg), 0);

    while (1)
    {
        printf("Nhap du lieu muon gui: ");
        fgets(buf, 256, stdin);

        // remove newline character from input
        buf[strcspn(buf, "\n")] = '\0';

        // if 'exit' then break
        if (strcmp(buf,"exit") == 0) break;

        ret = send(client, buf, strlen(buf), 0);
        if (ret == 0) break;
        
    }
    
    close(client);
    return 0;
}