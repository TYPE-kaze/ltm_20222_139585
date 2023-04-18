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

    char mssv[10];
    char hoten[64];
    char ngaySinh[12];
    float dtb;
    char buf[256];

    while (1)
    {
        printf("Nhap thong tin sinh vien: \n");
        printf("Nhap MSSV:\n");
        scanf("%s", mssv);
        getchar();

        printf("Nhap ho ten:\n");
        fgets(hoten, sizeof(hoten), stdin);
        hoten[strlen(hoten) - 1] = 0;

        printf("Nhap ngay sinh:\n");
        scanf("%s", ngaySinh);
        getchar();

        printf("Nhap Diem TB:\n");
        scanf("%f", &dtb);
        getchar();

        sprintf(buf,"%s %s %s %.2f", mssv, hoten, ngaySinh, dtb);
        printf("%s\n", buf);

        send(client, buf, strlen(buf), 0);

    }
    

    // char buf[MAX_LEN];

    //input student info
    // struct SinhVien sv;
   
    // printf("Nhap ho ten: ");
    // fgets(sv.hoTen, MAX_LEN, stdin);
    // sv.hoTen[strcspn(sv.hoTen, "\n")] = 0;

    // printf("Nhap MSSV: ");
    // fgets(sv.mssv, MAX_LEN, stdin);
    // sv.mssv[strcspn(sv.mssv, "\n")] = 0;

    // printf("Nhap ngay sinh: ");
    // fgets(sv.ngaySinh, MAX_LEN, stdin);
    // sv.ngaySinh[strcspn(sv.ngaySinh, "\n")] = 0;

    // printf("Nhap diem trung binh: ");
    // scanf("%f", &sv.diemTB);

    //send student info
    // ret = send(client, &sv, sizeof(sv), 0);
 
    // close(client);
    return 0;
}