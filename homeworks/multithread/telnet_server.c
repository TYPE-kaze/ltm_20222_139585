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
    printf("Listening of port 9000...\n");

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
    char buf[256];
    int is_login = false;

    char *not_login_msg = "Login using this pattern: <username> <password>\n";
    send(client, not_login_msg, strlen(not_login_msg), 0);

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Client %d disconnected.\n", client);
            break;
        }

        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        if (!is_login)
        {
            char username[128], passwd[128], tmp[128];
            ret = sscanf(buf, "%s%s%s", username, passwd, tmp);
            if (ret == 2)
            {
                // read user/pass tu file csdl
                FILE *f = fopen("csdl.txt", "r");
                char line[100];
                if (f == NULL)
                {
                    perror("Error reading csdl.txt");
                    return NULL;
                }
                // so sanh
                // TODO: this may be including even the ENTER char!? It's still correct because the request also inclde the ENTER char
                while (fgets(line, sizeof(line), f) != NULL)
                {
                    if (strcmp(buf, line) == 0)
                    {
                        is_login = true;
                        break;
                    }
                }
                fclose(f);
                // gui thong bao
                if (is_login)
                {
                    char *msg = "Welcome User, now you can start sending command.\n";
                    send(client, msg, strlen(msg), 0);
                }
                else
                {
                    char *msg = "Wrong username or password.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "Wrong Syntax. Please retry.\n";
                send(client, msg, strlen(msg), 0);
            }
            continue;
        }

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
            return NULL;
        }

        // Determine the size of the file
        fseek(file, 0L, SEEK_END);
        file_size = ftell(file);
        rewind(file);

        char result[file_size];
        // Read the file into the msg
        fread(result, file_size, 1, file);
        fclose(file);

        send(client, result, file_size, 0);
    }

    close(client);
}