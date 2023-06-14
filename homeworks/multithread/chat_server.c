#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

void *client_thread(void *);
int user[128];      // store fd of socket which is loggin
char *user_id[128]; // a level 2 poiter to an array of 128 level 1 pointer to a char/string
int num_user = 0;

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
    addr.sin_port = htons(8999);

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
    printf("Listening of port 8999...\n");

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    int is_login = false;
    int idx;
    char buf[256];

    char *msg = "Please use this format to login: client_id: <your_id>\n";
    send(client, msg, strlen(msg), 0);

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        // If connection shutdown
        if (ret <= 0)
        {
            printf("Client %d disconnected.\n", client);
            if (!is_login)
                break;

            if (idx != num_user - 1)
            {
                user[idx] = user[num_user - 1];
                user_id[idx] = user_id[num_user - 1];
            }
            num_user--;
            break;
        }

        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        // if not login
        if (!is_login)
        {
            char cmd[32], id[32], tmp[32];
            ret = sscanf(buf, "%s%s%s", cmd, id, tmp);

            // if 2 or more cmd or not client_id:
            if (ret != 2 && strcmp(cmd, "client_id:"))
            {
                char *msg = "Wrong syntax. Please try again...\n";
                send(client, msg, strlen(msg), 0);
                continue;
            }

            // if ID already registered
            int i = 0;
            for (; i < num_user; i++)
                if (!strcmp(user_id[i], id))
                    break;

            if (i < num_user)
            {
                char *msg = "ID already registered, try another one...\n";
                send(client, msg, strlen(msg), 0);
                continue;
            }

            // IF not thing wrong
            idx = num_user;
            user[num_user] = client;
            user_id[num_user] = malloc(strlen(id) + 1);
            strcpy(user_id[num_user], id);
            num_user++;
            is_login = true;

            char *msg = "Login Successful!\n`All <msg>` to chat all\n`<to_client_id> <msg>` to private chat\n";
            send(client, msg, strlen(msg), 0);
            continue;
        }

        // if login
        //   Time
        time_t t = time(NULL);
        struct tm *local_time = localtime(&t);
        char datetime_string[30];
        strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%d %I:%M:%S%p", local_time);

        // Check the msg format
        // client_id format: <client> <message>
        char to_client[32], msg[128];
        ret = sscanf(buf, "%s %[^\n]", to_client, msg);

        if (ret != 2)
        {
            char *msg = "Wrong syntax. Cant send to anyone...\n";
            send(client, msg, strlen(msg), 0);
            continue;
        }

        sprintf(buf, "%s %s: %s\n", datetime_string, user_id[idx], msg);

        if (!strcmp(to_client, "All"))
        {
            for (int k = 0; k < num_user; k++)
                if (user[k] != client)
                    send(user[k], buf, strlen(buf), 0);
            continue;
        }

        int j = 0;
        for (; j < num_user; j++)
        {
            if (!strcmp(user_id[j], to_client))
                break;
        }

        if (j == num_user)
        {
            char *msg = "No user with id like that. Please retry...\n";
            send(client, msg, strlen(msg), 0);
            continue;
        }

        send(user[j], buf, strlen(buf), 0);
    }

    close(client);
}