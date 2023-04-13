#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h> //contain IPPROTO_TCP
#include <arpa/inet.h> // contain inet_addr to convert addr string to in

int main() {
    //Create Socket
    char const *SERVER_IP = "127.0.0.1";
    int const PORT = 9090;
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr.sin_port = htons(PORT);   

    //connect
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1) {
        perror("connect() failed");
        return 1;
    }
    printf("connect to sever address %s success\n", SERVER_IP);

    //send a msg
    char *msg = "Hello from client\n";
    int ret = send(client, msg, strlen(msg), 0);
    if (ret == -1) {
        perror("send() failed");
        return 1;
    }
    printf("sending msg success\n");
    
    close(client);
    return 0;

}
