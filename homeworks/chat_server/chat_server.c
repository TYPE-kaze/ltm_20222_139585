#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

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
    addr.sin_port = htons(9090);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    printf("Listening on port 9090...\n");
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct client_info
    {
        char client_id[128];
        char client_name[128];
    };

    fd_set fdread;
    int clients[64];
    struct client_info client_infos[64];
    char *msg = "Please send your info in this format: <client_id>:<client_name>\n";
    int num_clients = 0;

    char buf[256];

    while (1)
    {
        FD_ZERO(&fdread);

        FD_SET(listener, &fdread);
        for (int i = 0; i < num_clients; i++)
            FD_SET(clients[i], &fdread);

        int ret = select(FD_SETSIZE, &fdread, NULL, NULL, NULL);

        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            if (client < FD_SETSIZE)
            {
                printf("New client connected %d\n", client);
                send(client, msg, strlen(msg), 0);

                // Thêm vào tập fdread
                clients[num_clients++] = client;
            }
            else
            {
                // Đã vượt quá số kết nối tối đa
                printf("Exceed the maximum client");
                close(client);
            }
        }

        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread))
            {
                int ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Xoa client ra khoi mang
                    continue;
                }
                // print tin nhan dc
                buf[ret] = 0;
                printf("Received from %d: %s", clients[i], buf);
                // kierm tra xem client da dang ky chua
                if (strlen(client_infos[i].client_id) == 0)
                {
                    printf("Client %d is not a registerd client\nChecking if info is in right format...\n", clients[i]);
                    // Kiem tra
                    if (strchr(buf, ':') == NULL)
                    { // fail
                        printf("The recieved info in not in right format. Exiting...\n\n");
                        send(clients[i], msg, strlen(msg), 0);
                    }
                    else
                    { // pass
                        printf("The recieved info in in right format\nRegistering the client...\n");
                        // save the id
                        char *token = strtok(buf, ":");
                        strcpy(client_infos[i].client_id, token);
                        // save the name
                        token = strtok(NULL, ":");
                        strcpy(client_infos[i].client_name, token);
                        printf("Client %d is registerd\n\n", clients[i]);
                    }
                }
                else // Tin nhan tu client da dang ky
                {
                    printf("sender: %s:%s\n", client_infos[i].client_id, client_infos[i].client_name);
                    printf("Send to other registerd clients...\n");
                    // tao msg cho cac client khac
                    // time
                    time_t t = time(NULL);
                    struct tm *local_time = localtime(&t);
                    char datetime_string[30];
                    strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%d %I:%M:%S%p", local_time);
                    // content
                    char temp_msg[256];
                    sprintf(temp_msg, "%s %s:%s", datetime_string, client_infos[i].client_id, buf);

                    for (int j = 0; j < num_clients; j++)
                    {
                        // neu la sender -> skip
                        if (clients[i] == clients[j])
                            continue;
                        // send
                        send(clients[j], temp_msg, strlen(temp_msg), 0);
                    }
                }
            }
    }

    close(listener);

    return 0;
}