#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

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

    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", client);

    // Truyen nhan du lieu
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;

    printf("%d bytes received\n", ret);

    int pos = 0;
    char computer_name[64];
    strcpy(computer_name, buf);
    pos = strlen(computer_name) + 1;

    printf("Computer name: %s\n", computer_name);   

    int num_drives = (ret - pos) / 3;

    for (int i = 0; i < num_drives; i++)
    {
        char drive_letter = buf[pos];
        pos++;
        unsigned short drive_size;
        memcpy(&drive_size, buf + pos, sizeof(drive_size));
        pos += sizeof(drive_size);

        printf("%c - %ud\n", drive_letter, drive_size);
    }

    close(client);
    close(listener);    

    return 0;
}