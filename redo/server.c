#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>


int main() {
    //create socket
    int socketFD = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);

    if(socketFD == -1) {
        perror("Socket Creation Faild!");
        return 1;
    }

    //create socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    addr.sin_port = htons(9000);

    //bind socket with address
    bind(socketFD,(struct sockaddr *) &addr, sizeof(addr));

    //change socket to 'waiting for connection' state
    listen(socketFD ,10);

    //accpect connection that is waiting 
    struct sockaddr_in clientAddr; //pointer for client'ID
    socklen_t addrlen = sizeof(clientAddr); //pointer for the length of clent'ID

    int client = accept(socketFD, (struct sockaddr *)&clientAddr, &addrlen);

    printf("IP: %s\n", inet_ntoa(clientAddr.sin_addr));

    return 0;
}