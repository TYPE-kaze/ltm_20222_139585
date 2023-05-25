#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <time.h>

#define MAX_CLIENTS 64

int main()
{

    int const PORT = 9000;

    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    printf("Listening on port %d...\n", PORT);
    if (listen(listen_fd, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listen_fd, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                sprintf(buf, "Xin chao. Hien dang co %d clients dang ket noi\n", nfds - 1);
                send(client, buf, strlen(buf), 0);
            }
            else
            {
                printf("Too many connections\n");
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected.\n", fds[i].fd);
                    close(fds[i].fd);

                    // Xoa phan tu i khoi mang
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];

                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    int client_fd = fds[i].fd;

                    // Chuan hoa string
                    //  Trim whitespace from the start of the string
                    char *start = buf;
                    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
                        start++;

                    // Trim whitespace from the end of the string
                    char *end = buf + strlen(buf) - 1;
                    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                        end--;

                    *(end + 1) = '\n';
                    *(end + 2) = '\0';

                    int capitalize_next = 1;
                    for (char *p = start; *p; p++)
                    {
                        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
                        {
                            capitalize_next = 1;
                        }
                        else if (capitalize_next)
                        {
                            *p = (*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p;
                            capitalize_next = 0;
                        }
                        else
                        {
                            *p = (*p >= 'A' && *p <= 'Z') ? (*p - 'A' + 'a') : *p;
                        }
                    }

                    send(client_fd, buf, strlen(buf), 0);
                }
            }
        }
    }

    close(listen_fd);

    return 0;
}
