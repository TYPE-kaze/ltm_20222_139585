#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child %d terminated.\n", pid);
}

int main(int argc, char *argv[])
{
    char *dir_path = argv[1];
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9001);

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

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client.\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d.\n", client);
        if (fork() == 0)
        {
            close(listener);

            char buf[256];
            DIR *dir;
            struct dirent *ent;
            if (!(dir = opendir(dir_path)))
            {

                perror("opendir() failed");
                return 1;
            }

            char *file_names = malloc(1); // allocate memory for an empty string
            file_names[0] = '\0';         // initialize the string to be empty
            int count = 0;

            // gui ds cac file cho client

            while (1)
            {
                if (!(ent = readdir(dir)))
                    break;

                if ((*ent).d_type == DT_REG)
                {
                    file_names = realloc(file_names, strlen(file_names) + strlen(ent->d_name) + 2 + 1);
                    strcat(file_names, (*ent).d_name); // append the file name to the string
                    strcat(file_names, "\r\n");        // add a space after the file name
                    count++;
                }
            }

            if (!count)
            {
                char *msg = "ERROR No files to download \r\n";
                send(client, msg, strlen(msg), 0);
                close(client);
                exit(0);
            }

            closedir(dir);

            sprintf(buf, "OK %d\r\n", count);
            send(client, buf, strlen(buf), 0);
            send(client, file_names, strlen(file_names), 0);
            send(client, "\r\n\r\n", 4, 0);

            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);

                if (buf[ret - 1] == '\n')
                    buf[ret - 1] = 0;

                // create the path: argv[1] + / + buf

                char path_to_file[512];
                sprintf(path_to_file, "%s/%s", dir_path, buf);

                FILE *f = fopen(path_to_file, "r");
                if (f == NULL)
                {
                    char *msg = "Cannot open file \r\n";
                    send(client, msg, strlen(msg), 0);
                    close(client);
                    exit(0);
                }

                // Lay kich thuoc file
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);

                sprintf(buf, "OK %ld\r\n", fsize);
                send(client, buf, strlen(buf), 0);

                while (!feof(f))
                {
                    int ret = fread(buf, 1, sizeof(buf), f);
                    if (ret <= 0)
                        break;
                    send(client, buf, ret, 0);
                }
                fclose(f);
                break;
            }

            close(client);
            exit(0);
        }

        close(client);
    }

    close(listener);

    return 0;
}