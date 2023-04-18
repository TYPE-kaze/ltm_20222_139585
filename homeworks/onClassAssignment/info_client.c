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
    // if (argc != 3)
    // {
    //     printf("Sai tham so.\n");
    //     return 1;
    // }

    // char const *destIP = argv[1];
    // int const destPort = atoi(argv[2]);


    // int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // struct sockaddr_in addr;
    // addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = inet_addr(destIP);
    // addr.sin_port = htons(destPort);

    // int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    // if (ret == -1)
    // {
    //     perror("connect() failed");
    //     return 1;
    // }

    //Nhap du lieu va tao buffer chua du lieu can du
    char name[64];
    int num_of_disk;
    char buf[256];
    struct DiskInfo {
        char name[1];
        unsigned short volume;
    };

    printf("Nhap ten may tinh:\n");
    fgets(name, sizeof(name), stdin);
    name[strlen(name) - 1] = 0;

    printf("Nhap so o dia:\n");
    scanf("%d", &num_of_disk);
    getchar();

    //Get disk info
    struct DiskInfo disks[num_of_disk];

    // for (int i = 0; i < num_of_disk; i++) {
    //     printf("Nhap ten ky tu o dia %d:" ,i);
    //     scanf("%s", &disks[i].name);
    //     getchar();

    //     printf("Nhap dung luong o dia %s (GB):", disks[i].name);
    //     scanf("%d", &disks[i].volume);
    //     getchar();
    //     printf("Disk Letter:%s - Volume:%d GB\n", disks[i].name, disks[i].volume);
    // }
    for (int i = 0; i < num_of_disk; i++) {
        printf("Nhap ten ky tu o dia %d:" ,i);
        scanf("%s", &disks[i].name);
        getchar();

        printf("Nhap dung luong o dia %s (GB):", disks[i].name);
        scanf("%d", &disks[i].volume);
        getchar();
        printf("Disk Letter:%s - Volume:%d GB\n", disks[i].name, disks[i].volume);
    }



    // sprintf(buf,"%s %s %s %.2f", mssv, hoten, ngaySinh, dtb);


    // while (1)
    // {
    //     printf("Nhap thong tin sinh vien: \n");
    //     printf("Nhap MSSV:\n");
    //     scanf("%s", mssv);
    //     getchar();

    //     printf("Nhap ho ten:\n");
    //     fgets(hoten, sizeof(hoten), stdin);
    //     hoten[strlen(hoten) - 1] = 0;

    //     printf("Nhap ngay sinh:\n");
    //     scanf("%s", ngaySinh);
    //     getchar();

    //     printf("Nhap Diem TB:\n");
    //     scanf("%f", &dtb);
    //     getchar();

    //     sprintf(buf,"%s %s %s %.2f", mssv, hoten, ngaySinh, dtb);
    //     printf("%s\n", buf);

    //     send(client, buf, strlen(buf), 0);

    // }
    
    return 0;
}