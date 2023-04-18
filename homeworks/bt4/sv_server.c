#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3)
    {
        printf("Sai tham so.\n");
        return 1;
    }

    int const destPort = atoi(argv[1]);
    char const *logFN = argv[2];
    // printf("port:%d\nhello_file_name:%s\nlogFN:%s\n", destPort, helloFN, logFN );

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(destPort);

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

    printf("Listening on port %d\n", destPort);

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    int client = accept(listener, 
        (struct sockaddr *)&client_addr, 
        &client_addr_len
    );

    printf("Accepted socket %d from IP: %s:%d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port)
    );

    char mssv[10];
    char hoten[64];
    char ngaySinh[12];
    float dtb;
    char buf[256];

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret<=0) break;

        buf[ret] = 0;

        memcpy(mssv, buf, 8);
        mssv[8] = 0;
        printf("MMSV: %s\n", mssv);

        int hoten_len = ret - 25;
        memcpy(hoten, buf + 9, hoten_len);
        hoten[hoten_len] = 0;
        printf("Ten: %s\n", hoten);

        memcpy(ngaySinh, buf + hoten_len + 10, 10);
        ngaySinh[10] = 0;
        printf("Ngay sinh: %s\n", ngaySinh);

        dtb = atof(buf + hoten_len + 21);
        printf("DTB: %.2f\n", dtb);

        //Time
        time_t t = time(NULL);
        struct tm *local_time = localtime(&t);
        char datetime_string[20];
        strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%d %H:%M:%S", local_time);
        
        //log entry
        char entry[512];
        sprintf(entry, "%s %s %s", inet_ntoa(client_addr.sin_addr), datetime_string, buf);

        //print log entry
        printf("%s\n", entry);

        //log
        FILE *file;
        file = fopen(logFN, "a+");
        fprintf(file,"%s\n", entry);
        fclose(file);
    }

    close(client);
    close(listener);
    return 0;
}