#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <stdbool.h>

void *client_thread(void *);
void *waiter_thread();
int alone_fd = -1;
pthread_t waiter_thread_id;

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
    printf("Linstening on port 9000...\n");

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        if (alone_fd == -1)
        {
            alone_fd = client;
            char *msg = "Please wait for a partner...\n";
            pthread_create(&waiter_thread_id, NULL, waiter_thread, NULL);
            pthread_detach(waiter_thread_id);

            send(client, msg, strlen(msg), 0);
            continue;
        }

        int couple[2];
        couple[0] = alone_fd;
        couple[1] = client;

        pthread_cancel(waiter_thread_id);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, couple);
        pthread_detach(thread_id);

        alone_fd = -1;
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int user1 = ((int *)param)[0];
    int user2 = ((int *)param)[1];
    bool is_chating = true;

    struct pollfd fds[2];
    int nfds = 2;

    fds[0].fd = user1;
    fds[0].events = POLLIN;
    fds[1].fd = user2;
    fds[1].events = POLLIN;

    char buf[256] = "A couple is found.\nYou can now start chatting with each other\n";
    send(user1, buf, strlen(buf), 0);
    send(user2, buf, strlen(buf), 0);

    while (1)
    {
        if (!is_chating)
            break;

        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        printf("ret = %d\n", ret);

        for (int i = 0; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                int partner_fd = fds[i].fd == user1 ? user2 : user1;
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected.\n", fds[i].fd);
                    char *msg = "Your partner is disconnected!\nbye bye :)\n";
                    send(partner_fd, msg, strlen(msg), 0);
                    is_chating = false;
                }
                else
                {
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    send(partner_fd, buf, strlen(buf), 0);
                }
            }
    }

    close(user1);
    close(user2);
}

void *waiter_thread()
{
    char buf[256];

    while (1)
    {
        int ret = recv(alone_fd, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("alone_client_fd %d dicconected before a partner arrive :<\n", alone_fd);
            break;
        }

        buf[ret] = 0;
        printf("Received from %d: %s\n", alone_fd, buf);

        char *msg = "Please wait for a partner...\n";
        send(alone_fd, msg, strlen(msg), 0);
    }

    close(alone_fd);
    alone_fd = -1;
}