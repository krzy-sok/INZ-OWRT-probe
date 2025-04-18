// Custom ping implementation in C
// For purpose of probing hosts in network and measuring its rtt
// rtt, ip


// reference https://www.geeksforgeeks.org/ping-in-c/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>


#define PING_PKT_S  64;
// global vars not in define for ability to set them from call arguments
// how many packets should be send to a single host in each cycle
int PING_NUM;


// since icmphdr does not include data
// and icmp struct does not send on modern system
struct icmp_pkt
{
    struct icmphdr hdr;
    char msg[64 - sizeof(struct icmphdr)];
};

// Calculate the checksum (RFC 1071)
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char **argv)
{
    struct sockaddr_in dst;

    if(argc < 2){
        printf("Ip address needed\n");
        exit(-1);
    }
    // check if given ip is valid
    if (inet_aton(argv[1], &dst.sin_addr) == 0){
        perror("Invalid ip addr\n");
        exit(-1);
    }

    // create IPPROTO_ICMP socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    printf("created socket\n");


    close(sock);
    return 0;
}
