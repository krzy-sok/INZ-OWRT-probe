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
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>


#define PING_PKT_S 64
#define TIMEOUT_SEC 4
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

int send_ping(int sock, struct sockaddr_in dst, char* addr)
{
    int ttl = 64;
    unsigned char packet_buffer[PING_PKT_S];
    struct icmp_pkt *packet = (struct icmp_pkt *) packet_buffer;

    struct timespec t_sent, t_recived;
    struct timeval tv_timeout;
    tv_timeout.tv_sec = TIMEOUT_SEC;
    tv_timeout.tv_usec = 0;

    // set ttl
    if (setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        printf("\nSetting socket options to TTL failed!\n");
        return -1;
    } else {
        printf("\nSocket set to TTL...\n");
    }

    // set timeout on recive
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_timeout, sizeof tv_timeout);

    // compose packet
    // set header
    packet->hdr.type = ICMP_ECHO;
    packet->hdr.code = 0;
    packet->hdr.checksum = 0;
    packet->hdr.un.echo.id = getpid();

    // set data portion to acceding numbers
    // for (long unsigned int i = 0; i < sizeof(packet->msg) - 1; i++){
    //     packet->msg[i] = i + "0";
    // }

    // set checksum
    packet->hdr.checksum = checksum(&packet, sizeof(struct icmp_pkt));

    // CCLOCK_REALTIME is considered undeclared
    // but its value is 0 as per https://codebrowser.dev/glibc/glibc/sysdeps/unix/sysv/linux/bits/time.h.html
    clock_gettime(0, &t_sent);

    // send packet
    // fails to send - suspect big endian
    // hotn entire buffer or do just the header since it worked
    printf("packet type %d, packet code %d\n", packet->hdr.type, packet->hdr.code);
    int sent_res = sendto(sock, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr));
    if (sent_res < 0){
        printf("Failed to send packet! %d\n", sent_res);
        return -1;
    }

    // prepare buffer
    unsigned char reply_buffer[128];
    socklen_t dst_len = sizeof(struct sockaddr);
    // receive reply
    int recv_res = recvfrom(sock, &reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&dst, &dst_len);
    if (recv_res <0){
        printf("Failed to receive packet!%d\n", recv_res);
        return -1;
    }

    clock_gettime(0, &t_recived);
    double time_elapsed = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;
    double rtt = (t_recived.tv_sec - t_sent.tv_sec) * 1000 + time_elapsed;

    struct icmphdr *recv_hdr = (struct icmphdr *)reply_buffer;
    if (recv_hdr->type != 0 && recv_hdr->code!=0){
        printf("Not echo reply\n");
        return -1;
    }
    printf("%d bytes from (ip: %s) rtt = %f ms.\n", PING_PKT_S, addr, rtt);
    return 0;
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

    send_ping(sock, dst, argv[1]);

    close(sock);
    return 0;
}
