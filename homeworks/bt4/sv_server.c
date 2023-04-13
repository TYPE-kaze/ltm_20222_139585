#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LEN 256
struct SinhVien {
    char mssv[MAX_LEN];
    char hoTen[MAX_LEN];
    char ngaySinh[MAX_LEN];
    float diemTB;
};

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
    
    //Sv info
    struct SinhVien sv;
    int ret = recv(client, &sv, sizeof(sv), 0);
    char sv_string[256];
    sprintf(sv_string,"%s %s %.2f",sv.hoTen, sv.ngaySinh, sv.diemTB);

    //Time
    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    char datetime_string[20];
    strftime(datetime_string, sizeof(datetime_string), "%Y-%m-%d %H:%M:%S", local_time);
    
    //log entry
    char entry[512];
    sprintf(entry, "%s %s %s", inet_ntoa(client_addr.sin_addr), datetime_string, sv_string);
    printf("%s\n", entry);

    //log
    FILE *file;
    file = fopen(logFN, "a+");
    fprintf(file,"%s\n", entry);
    fclose(file);

    close(client);
    close(listener);
    return 0;
}