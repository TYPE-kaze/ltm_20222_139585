#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

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
    addr.sin_port = htons(3001);

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
    printf("Listening on port 3001...\n");

    int num_processes = 8;
    for (int i = 0; i < num_processes; i++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                char buf[256];
                char *request = NULL;
                int size = 0;

                int client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);

                while (1)
                {
                    int ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                        continue;

                    request = realloc(request, size + ret);
                    memcpy(request + size, buf, ret);
                    size += ret;

                    if (strstr(request, "\r\n\r\n") != NULL)
                        break;
                }

                printf("%d bytes from client: %s", size, request);
                free(request);

                strcpy(buf, "HTTP/1.1 200 OK\r\nContentType: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>");
                send(client, buf, strlen(buf), 0);

                close(client);
            }
            exit(0);
        }
    }

    getchar();

    close(listener);
    killpg(0, SIGKILL);

    return 0;
}