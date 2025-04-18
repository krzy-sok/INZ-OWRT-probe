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
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>


#define PING_PKT_S 64
#define TIMEOUT_SEC 2
// global vars not in define for ability to set them from call arguments
// how many packets should be send to a single host in each cycle
int PING_NUM;


// since icmphdr does not include data
// and icmp struct does not send on modern system
struct icmp_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
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

int send_ping(int sock, struct sockaddr_in dst)
{
    int ttl = 64;
    unsigned char packet_buffer[PING_PKT_S];
    struct icmp_pkt *packet = (struct icmp_pkt *) packet_buffer;

    // set header
    packet->hdr.type = 0;
    packet->hdr.code = 0;
    packet->hdr.checksum = 0;

    struct timespec t_sent, t_recived;
    struct timeval *tv_timeout;
    tv_timeout->tv_sec = TIMEOUT_SEC;
    tv_timeout->tv_usec = 0;

    // set ttl
    if (setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        printf("\nSetting socket options to TTL failed!\n");
        return;
    } else {
        printf("\nSocket set to TTL...\n");
    }

    // set timeout on recive
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)tv_timeout, sizeof tv_timeout);

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

    send_ping(sock, dst);

    close(sock);
    return 0;
}
