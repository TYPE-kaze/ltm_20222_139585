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
    int listener_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_fd == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    printf("Listening on port 9000...\n");
    if (listen(listener_fd, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listener_fd;
    fds[0].events = POLLIN;

    char buf[256];

    int users[64];      // Mang socket client da dang nhap
    char *user_ids[64]; // Mang luu tru id cua client da dang nhap
    int num_users = 0;  // So client da dang nhap

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
            int client = accept(listener_fd, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
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
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", fds[i].fd, buf);
                    int client_fd = fds[i].fd;

                    // Kiem tra nguoi dung nay co dang nhap khong
                    int j = 0;
                    while (j < num_users)
                    {
                        if (client_fd == users[j])
                            break;
                        j++;
                    }

                    // neu j = num_user -> fd nay chua dang nhap
                    // else da dang nhap
                    // Chua dang nhap
                    // Xu ly cu phap yeu cau dang nhap
                    if (j == num_users)
                    {

                        char cmd[32], id[32], tmp[32];
                        ret = sscanf(buf, "%s%s%s", cmd, id, tmp);
                        if (ret == 2)
                        {
                            if (strcmp(cmd, "client_id:") == 0)
                            {
                                char *msg = "Dung cu phap. Gui tin nhan.\nGui tin nhan ca nhan theo format sau: [<client_id>] <message>\n";
                                send(client_fd, msg, strlen(msg), 0);

                                int k = 0;
                                for (; k < num_users; k++)
                                    if (strcmp(user_ids[k], id) == 0)
                                        break;

                                if (k < num_users)
                                {
                                    char *msg = "ID da ton tai. Yeu cau nhap lai.\n";
                                    send(client_fd, msg, strlen(msg), 0);
                                }
                                else
                                {
                                    users[num_users] = client_fd;
                                    user_ids[num_users] = malloc(strlen(id) + 1);
                                    strcpy(user_ids[num_users], id);
                                    num_users++;
                                }
                            }
                            else
                            {
                                char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                                send(client_fd, msg, strlen(msg), 0);
                            }
                        }
                        else
                        {
                            char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                            send(client_fd, msg, strlen(msg), 0);
                        }
                    }
                    else
                    {
                        // Da dang nhap
                        char sendbuf[512];

                        time_t t = time(NULL);
                        struct tm *local_time = localtime(&t);
                        char datetime_string[30];
                        strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%d %I:%M:%S%p", local_time);

                        // Check the msg format
                        // client_id format: [<client_id>] <message>
                        char to_client_id[32], msg[256];
                        // ret = sscanf(buf, "%s%s", to_client_id, msg);
                        // char *ptr = strchr(to_client_id, ']');

                        if (buf[0] == '[' && strchr(buf, ']') != NULL)
                        {
                            // send to one
                            // find the fd
                            // *ptr = 0;
                            char *token = strtok(buf, "[]");
                            strcpy(to_client_id, token);
                            // save the name
                            token = strtok(NULL, "[]");
                            strcpy(msg, token);
                            printf("client_id to send to: %s\n", to_client_id);

                            int h = 0;
                            for (; h < num_users; h++)
                            {
                                if (strcmp(user_ids[h], to_client_id) == 0)
                                {
                                    sprintf(sendbuf, "%s %s: %s", datetime_string, user_ids[j], msg);
                                    send(users[h], sendbuf, strlen(sendbuf), 0);
                                    break;
                                }
                            }

                            if (h == num_users)
                            {
                                char *err_msg = "Khong tim thay nguoi dung. Yeu cau nhap lai.\n";
                                send(users[j], err_msg, strlen(err_msg), 0);
                            }
                        }
                        else
                        {
                            // Send all
                            sprintf(sendbuf, "%s %s: %s", datetime_string, user_ids[j], buf);

                            for (int k = 0; k < num_users; k++)
                                if (users[k] != client_fd)
                                    send(users[k], sendbuf, strlen(sendbuf), 0);
                        }
                    }
                }
            }
    }

    close(listener_fd);

    return 0;
}