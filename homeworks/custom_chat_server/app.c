#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <ctype.h>

#define MAX_CLIENTS 64
void *join_handler(int client, char *arg);
void *forward_all(int client, char *msg);
void *forward_to(int client, char *reciver, char *msg);

int users[64];      // Mang socket client da dang nhap
char *user_ids[64]; // Mang luu tru id cua client da dang nhap
int num_users = 0;  // So client da dang nhap
char buf[256];

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

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        printf("ret = %d\n", ret);

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                char *msg = "Connected\n";
                send(client, msg, strlen(msg), 0);
            }
            else
            {
                printf("Too many connections\n");
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    printf("Client %d disconnected.\n", fds[i].fd);
                    close(fds[i].fd);

                    int j = 0;
                    while (j < num_users)
                    {
                        if (fds[i].fd == users[j])
                            break;
                        j++;
                    }

                    if (j < num_users)
                    {
                        if (j != num_users - 1)
                        {
                            users[j] = users[num_users - 1];
                            user_ids[j] = user_ids[num_users - 1];
                        }
                        num_users--;
                    }

                    // Xoa phan tu i khoi mang
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];

                    nfds--;
                    i--;
                }
                else
                {
                    int client = fds[i].fd;
                    buf[ret] = 0;
                    int is_err = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    // Kiem tra lenh
                    // neo ko bao loi
                    // neu la lenh co ho tro -> xu ly
                    char verb[6];
                    char arg[256];
                    ret = sscanf(buf, "%s%[^\0]", verb, arg);
                    printf("VERB: %s\n, arg: %s\n", verb, arg);
                    if (strcmp(verb, "JOIN") == 0)
                    {
                        char temp1[128];
                        char temp2[128];
                        ret = sscanf(arg, "%s%s", temp1, temp2);
                        if (ret == 1)
                        {
                            join_handler(client, arg);
                        }
                        else
                            is_err = 1;
                    }
                    else if (strcmp(verb, "MSG") == 0)
                    {
                        if (ret == 2)
                        {
                            forward_all(client, arg);
                        }
                        else
                            is_err = 1;
                    }
                    else if (strcmp(verb, "PMSG") == 0)
                    {
                        char to_nickname[64];
                        char msg[128];
                        char temp[64];
                        ret = sscanf(arg, "%s%s", to_nickname, msg, temp);
                        if (ret == 2)
                        {
                            forward_to(client, to_nickname, msg);
                        }
                        else
                            is_err = 1;
                    }
                    else
                    {
                        // VERB Not handled
                    }

                    if (is_err == 1)
                    {
                        char *msg = "999 UNKNOWN ERROR\n";
                        send(client, msg, strlen(msg), 0);
                    }
                }
            }
    }

    close(listener);

    return 0;
}

void *forward_to(int client, char *to_nickname, char *msg)
{
    // find if to_nickname exsit
    int i = 0;
    for (; i < num_users; i++)
        if (strcmp(to_nickname, user_ids[i]) == 0)
            break;

    if (i == num_users)
    {
        char *msg = "202 UNKNOWN NICKNAME\n";
        send(client, msg, strlen(msg), 0);
        return 0;
    }
    int to_recv = i;
    // find sender name
    i = 0;
    for (; i < num_users; i++)
        if (client == users[i])
            break;
    char *nickname = user_ids[i];

    // send to that user
    sprintf(buf, "PMSG %s %s\n", nickname, msg);
    send(to_recv, buf, strlen(buf), 0);

    char *res = "100 OK\n";
    send(client, res, strlen(res), 0);
}

void *forward_all(int client, char *msg)
{
    int i = 0;
    for (; i < num_users; i++)
        if (client == users[i])
            break;

    char *nickname = user_ids[i];
    sprintf(buf, "MSG %s %s\n", nickname, msg);
    for (int j = 0; j < num_users; j++)
        if (users[j] != client)
            send(users[j], buf, strlen(buf), 0);
    char *res = "100 OK\n";
    send(client, res, strlen(res), 0);
}

int only_lower_and_digit(char *s)
{
    while (*s != '\0')
    {
        if (!islower(*s) && !isdigit(*s))
            return 1;
        s++;
    }
    return 0;
};

void *join_handler(int client, char *nickname)
{
    // TODO handle the case where user already login
    int k = 0;
    for (; k < num_users; k++)
        if (strcmp(user_ids[k], nickname) == 0)
            break;

    // if nickname already exsit
    if (k < num_users)
    {
        char *msg = "200 NICKNAME IN USE\n";
        send(client, msg, strlen(msg), 0);
        return 0;
    }
    // if the string contain invaid char
    if (only_lower_and_digit(nickname) == 1)
    {
        char *msg = "201 INVALID NICK NAME\n";
        send(client, msg, strlen(msg), 0);
        return 0;
    };
    // if ok

    users[num_users] = client;
    user_ids[num_users] = malloc(strlen(nickname) + 1);
    strcpy(user_ids[num_users], nickname);
    num_users++;

    char *msg = "100 OK\n";
    send(client, msg, strlen(msg), 0);
};