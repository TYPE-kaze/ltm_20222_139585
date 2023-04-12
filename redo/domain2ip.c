#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    char *nodename = argv[1];
    char *servname = "http";
    struct addrinfo *res, *p, hints;
    // hints.ai_family = AF_INET;
    int isAddrGet;

    //isAddrGet == 0 => success
    isAddrGet = getaddrinfo(nodename, servname, NULL, &res);
    if (isAddrGet != 0) {
        printf("Failed to get IP %s\n", gai_strerror(isAddrGet));
        return 1;
    };

    printf("Successful getting IP\n");

    p = res;
    while (p != NULL) {
        //IPv4
        if (p->ai_family == AF_INET) {
            printf("IPv4: %s\n",inet_ntoa(((struct sockaddr_in *)(res->ai_addr))->sin_addr));
        }
        //IPv6
        char buf[64];
        if (p->ai_family == AF_INET6) {
            printf("IPv6: %s\n", inet_ntop(AF_INET6,&((struct sockaddr_in6 *)(p->ai_addr))->sin6_addr, buf,INET6_ADDRSTRLEN));
        }
        p = p->ai_next;
    }
    return 0;
}