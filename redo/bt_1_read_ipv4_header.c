#include <stdio.h>

int main() {
    unsigned char packet_bytes[] = {
        0x45, 0x00, 0x02, 0x04, 0x0a, 0x8c, 0x40, 0x00,
        0x80, 0x06, 0x00, 0x00, 0x0a, 0x5a, 0x4c, 0x96,
        0x34, 0x56, 0x44, 0x2e
    };

    int version, ihl, total_length;

    //read and print ip version
    version = packet_bytes[0] >> 4;
    printf("IP Version: %d \n", version);

    //read ihl, result from (header length) / 32 bits , aka the n number of '32 bit'
    ihl = packet_bytes[0] & 0x0f;
    printf("IHL: %d bits\n", ihl * 32);

    //read total length
    total_length = packet_bytes[2] * 256 + packet_bytes[3];
    printf("Total packet length: %d bytes\n", total_length);

    //read source IP address
    printf("Sourcr IP address: %u.%u.%u.%u\n", 
         packet_bytes[12], 
         packet_bytes[13], 
         packet_bytes[14], 
         packet_bytes[15]
    );

    //read dest IP address
        printf("Dest. IP address: %u.%u.%u.%u\n", 
         packet_bytes[16], 
         packet_bytes[17], 
         packet_bytes[18], 
         packet_bytes[19]
    );
}