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

    int users[64];     // Mang socket client da dang nhap
    int num_users = 0; // So client da dang nhap

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
            int client_fd = accept(listener_fd, NULL, NULL);
            if (nfds < MAX_CLIENTS)
            {
                printf("New client connected: %d\n", client_fd);
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
                char *msg = "Login using this pattern: <username> <password>\n";
                send(client_fd, msg, strlen(msg), 0);
            }
            else
            {
                printf("Too many connections\n");
                close(client_fd);
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
                        char username[128], passwd[128], tmp[128];
                        ret = sscanf(buf, "%s%s%s", username, passwd, tmp);
                        if (ret == 2)
                        {
                            // doc user/pass tu file csdl
                            FILE *f = fopen("csdl.txt", "r");
                            char line[100];
                            int is_user = 0;
                            if (f == NULL)
                            {
                                perror("Error reading csdl.txt");
                                return (-1);
                            }
                            // so sanh
                            // TODO: this may be including even the ENTER char!?
                            while (fgets(line, sizeof(line), f) != NULL)
                            {
                                if (strcmp(buf, line) == 0)
                                {
                                    is_user = 1;
                                    break;
                                }
                            }
                            fclose(f);
                            // gui thong bao
                            if (is_user)
                            {
                                users[num_users] = client_fd;
                                num_users++;
                                char *msg = "Welcome User, now you can start sending command.\n";
                                send(client_fd, msg, strlen(msg), 0);
                            }
                            else
                            {
                                char *msg = "Wrong username or password.\n";
                                send(client_fd, msg, strlen(msg), 0);
                            }
                        }
                        else
                        {
                            char *msg = "Nhap sai. Yeu cau nhap lai.\n";
                            send(client_fd, msg, strlen(msg), 0);
                        }
                    }
                    else // da dang nhap, bat dau nhan lenh
                    {
                        char command[256];
                        if (buf[strlen(buf) - 1] == '\n')
                            buf[strlen(buf) - 1] = 0;

                        sprintf(command, "%s > out.txt 2> out.txt ", buf);
                        system(command);

                        FILE *file;
                        long file_size;

                        file = fopen("out.txt", "r");
                        if (file == NULL)
                        {
                            perror("Error opening file");
                            return (-1);
                        }

                        // Determine the size of the file
                        fseek(file, 0L, SEEK_END);
                        file_size = ftell(file);
                        rewind(file);

                        char result[file_size];
                        // Read the file into the msg
                        fread(result, file_size, 1, file);
                        fclose(file);

                        send(client_fd, result, file_size, 0);
                    }
                }
            }
    }

    close(listener_fd);

    return 0;
}