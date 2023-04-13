#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

//Tạo server nhận thông điệp từ netcat client

int main() {
    int const PORT_NUM = 9090;
    //create socket
    int listener = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);

    if(listener == -1) {
        perror("socket() Failed!");
        return 1;
    }

    //create socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    addr.sin_port = htons(PORT_NUM);

    //bind socket with address
    if (bind(listener,(struct sockaddr *) &addr, sizeof(addr))) {
        perror("bind() Failed!");
        return 1;
    }
    
    printf("Listening on port %d\n", PORT_NUM);
    //change socket to 'waiting for connection' state
    if (listen(listener ,10)) {
        perror("listen() Failed!");
        return 1;
    }

    //accpect connection that is waiting 
    struct sockaddr_in clientAddr; //pointer for client'ID
    socklen_t addrlen = sizeof(clientAddr); //pointer for the length of clent'ID

    int client = accept(listener, (struct sockaddr *)&clientAddr, &addrlen);

    printf("Accept Socket %d from address: %s:%d\n",
        client, 
        inet_ntoa(clientAddr.sin_addr), 
        ntohs(clientAddr.sin_port)
    );

    //receive ncat client send msg
    char buf[256];
    char *request = NULL;
    int size = 0;
    while (1) {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret == 0) break; // may be not needed

        request = realloc(request, size + ret);
        memcpy(request + size, buf, ret);

        if (ret < sizeof(buf)) buf[ret] = 0;
        size += ret;

        if(strstr(request, "\r\n\r\n") != NULL) break;
    }
    printf("%d bytes recieved from browser: \n%s",size, request);
    free(request);

    //Give a response to browser
    strcpy(buf, "HTTP/1.1 200 OK\r\nContentType: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>");
    send(client,buf, strlen(buf),0);

    printf("Connection closed!");
    close(client);
    close(listener);
    return 0;
}