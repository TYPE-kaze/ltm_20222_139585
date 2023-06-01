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
#include <time.h>

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child %d terminated.\n", pid);
}

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
    addr.sin_port = htons(10010);

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
    printf("Listening on port 10010...\n");

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client.\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted: %d.\n", client);

        char *wlc_msg = "syntax: GET_TIME [format]\nformat is any combination of 'dd', 'mm' andd 'yyyy'\n";
        send(client, wlc_msg, strlen(wlc_msg), 0);

        if (fork() == 0)
        {
            close(listener);
            char buf[256];
            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);

                char cmd[32], format[32], tmp[256];
                if (sscanf(buf, "%s%s%s", cmd, format, tmp) == 2)
                {
                    if (strcmp(cmd, "GET_TIME"))
                    {
                        // neu sai
                        char *msg = "Wrong command, Try again...\n";
                        send(client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        // neu dung
                        // dd/mm/yyyy
                        char *formatTokens[3];
                        char *token = strtok(format, "/");
                        formatTokens[0] = token;

                        int i = 1;
                        while (token != NULL && i < 5)
                        {
                            token = strtok(NULL, "/");
                            formatTokens[i] = token;
                            i++;
                        }

                        if (i != 4)
                        {
                            // neu co token thu 4 hoan it hon 3 token
                            char *msg = "Wrong format, Try again...\n";
                            send(client, msg, strlen(msg), 0);
                        }
                        else
                        {
                            // neu chi co 3 token
                            // check of follow dd//mm//yyyy
                            char time_format_str[8];
                            int is_correct = 1;

                            // TODO: There is a weird behaviour when trying to loop in a loop in a child process, workaround by using lots of if else
                            if (strcmp(formatTokens[0], "dd") == 0)
                            {
                                time_format_str[0] = '%';
                                time_format_str[1] = 'd';
                                time_format_str[2] = '/';

                                if (strcmp(formatTokens[1], "mm") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'm';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "yyyy") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'Y';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else if (strcmp(formatTokens[1], "yyyy") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'Y';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "mm") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'm';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else
                                {
                                    is_correct = 0;
                                }
                            }
                            else if (strcmp(formatTokens[0], "mm") == 0)
                            {
                                time_format_str[0] = '%';
                                time_format_str[1] = 'm';
                                time_format_str[2] = '/';

                                if (strcmp(formatTokens[1], "dd") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'd';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "yyyy") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'Y';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else if (strcmp(formatTokens[1], "yyyy") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'Y';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "dd") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'd';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else
                                {
                                    is_correct = 0;
                                }
                            }
                            else if (strcmp(formatTokens[0], "yyyy") == 0)
                            {
                                time_format_str[0] = '%';
                                time_format_str[1] = 'Y';
                                time_format_str[2] = '/';

                                if (strcmp(formatTokens[1], "dd") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'd';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "mm") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'm';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else if (strcmp(formatTokens[1], "mm") == 0)
                                {
                                    time_format_str[3] = '%';
                                    time_format_str[4] = 'm';
                                    time_format_str[5] = '/';

                                    if (strcmp(formatTokens[2], "dd") == 0)
                                    {
                                        time_format_str[6] = '%';
                                        time_format_str[7] = 'd';
                                    }
                                    else
                                    {
                                        is_correct = 0;
                                    }
                                }
                                else
                                {
                                    is_correct = 0;
                                }
                            }
                            else
                            {
                                is_correct = 0;
                            };
                            if (is_correct)
                            {
                                time_format_str[8] = 0;
                                printf("time format string: %s\n", time_format_str);

                                time_t t = time(NULL);
                                struct tm *local_time = localtime(&t);
                                char datetime_string[16];
                                strftime(datetime_string, sizeof(datetime_string), time_format_str, local_time);
                                int end = strlen(datetime_string);
                                datetime_string[end] = '\n';
                                datetime_string[end + 1] = '\0';
                                send(client, datetime_string, strlen(datetime_string), 0);
                            }
                            else
                            {
                                char *msg = "Wrong date syntax, Try again...\n";
                                send(client, msg, strlen(msg), 0);
                            }
                        }
                    }
                }
                else
                {
                    char *msg = "Wrong syntax, Try again...\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            close(client);
            exit(0);
        }

        close(client);
    }

    close(listener);

    return 0;
}