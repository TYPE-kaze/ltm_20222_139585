#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

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

    printf("Listening on port 9000\n");

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

    //set 9 first 9 byte to 'r', a random char 
    char buf[128];
    int ret;
    int counter = 0;
    memset(buf, 'r', 9);

    while (1)
    {
        //recvive 
        ret = recv(client, buf + 9, sizeof(buf) - 9 , 0);
        if (ret <= 0) break;

        buf[ret] = '\0';
        printf("recieved: %s\n",buf);
        char *pos = buf;
        while ((pos = strstr(pos, "0123456789")) != NULL) {
            counter++;
            pos += strlen("0123456789");
        }

        memmove(buf, &buf[ret - 9], 9);
        printf("buffer after move: %s\n",buf);
    }

    printf("The number of time '123456789' appear is: %d\n", counter);

    close(client);
    close(listener);
    return 0;
}