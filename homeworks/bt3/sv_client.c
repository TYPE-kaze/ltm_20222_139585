#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_LEN 256

struct SinhVien {
    char mssv[MAX_LEN];
    char hoTen[MAX_LEN];
    char ngaySinh[MAX_LEN];
    float diemTB;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Sai tham so.\n");
        return 1;
    }

    char const *destIP = argv[1];
    int const destPort = atoi(argv[2]);

    // printf("ip: %s\nport:%d\n", destIP, destPort);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(destIP);
    addr.sin_port = htons(destPort);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[MAX_LEN];

    //input student info
    struct SinhVien sv;
   
    printf("Nhap ho ten: ");
    fgets(sv.hoTen, MAX_LEN, stdin);
    sv.hoTen[strcspn(sv.hoTen, "\n")] = 0;

    printf("Nhap MSSV: ");
    fgets(sv.mssv, MAX_LEN, stdin);
    sv.mssv[strcspn(sv.mssv, "\n")] = 0;

    printf("Nhap ngay sinh: ");
    fgets(sv.ngaySinh, MAX_LEN, stdin);
    sv.ngaySinh[strcspn(sv.ngaySinh, "\n")] = 0;

    printf("Nhap diem trung binh: ");
    scanf("%f", &sv.diemTB);

    //send student info
    ret = send(client, &sv, sizeof(sv), 0);
 
    close(client);
    return 0;
}