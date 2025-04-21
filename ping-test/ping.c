// Custom ping implementation in C
// For purpose of probing hosts in network and measuring its rtt
// rtt, ip


// reference https://www.geeksforgeeks.org/ping-in-c/

// reference for raw eth https://hacked10bits.blogspot.com/2011/12/sending-raw-ethernet-frames-in-6-easy.html
// and https://gist.github.com/austinmarton/2862515

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
#define ODDBYTE(v) htons((unsigned short)(v) <<8)
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

// display contents of an array
// for comparing built and received packets with capture from wireshark
void dump(unsigned char *data, int size)
{
    for (int i = 0; i < 10; ++i)  // Output headings 0..9
        printf("\t%d", i);
    putchar('\n');

    for (int i = 0; i < size; i++)
    {
        if (i % 10 == 0)
            printf("%d", i / 10);   // Consider outputting i?
        printf("\t%02x", data[i]);
        if (i % 10 == 9)
            putchar('\n');
    }
    if (size % 10 != 0)
        putchar('\n');  // Finish current/partial line
    putchar('\n');      // Optional blank line
}

// Calculate the checksum (RFC 1071)
unsigned short in_cksum(
    const unsigned short *addr, register int len, unsigned short csum
)
{
	register int nleft = len;
	const unsigned short *w = addr;
	register unsigned short answer;
	register int sum = csum;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += ODDBYTE(*(unsigned char *)w); /* le16toh() may be unavailable on old systems */

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

double send_ping(int sock, struct sockaddr_in dst, char* addr)
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
    packet->hdr.checksum = in_cksum((unsigned short *)packet, sizeof(packet_buffer), 0 );

    // CCLOCK_REALTIME is considered undeclared
    // but its value is 0 as per https://codebrowser.dev/glibc/glibc/sysdeps/unix/sysv/linux/bits/time.h.html
    clock_gettime(0, &t_sent);

    // send packet
    // fails to send - suspect big endian
    // hotn entire buffer or do just the header since it worked
    printf("socket desc: %d", sock);
    printf("packet type %d, packet code %d\n", packet->hdr.type, packet->hdr.code);
    int sent_res = sendto(sock, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr));
    if (sent_res < 0){
        printf("Failed to send packet! %d\n", sent_res);
        printf("socaddr: %d\n\n", dst.sin_addr.s_addr);
        dump(packet_buffer, PING_PKT_S);
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
    dump(reply_buffer, recv_res);
    return rtt;
}

int handle_list(int list_len,char **argv)
{
    // create IPPROTO_ICMP socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    printf("created socket\n");


    // convert str to inet address
    struct sockaddr_in dst[list_len];
    printf("sending ping to %d domains\n", list_len);
    // loop tough list of given ips
    for(int i = 0; i<list_len; i++)
    {
        // skip program name
        if (inet_aton(argv[i+1], &dst[i].sin_addr) == 0){
            printf("Invalid ip addr on position %d\n", i);
            exit(-1);
        }
        else{
            printf("address: %s\n", argv[i+1]);
            send_ping(sock, dst[i], argv[i+1]);
        }
    }
    close(sock);
    return 0;
}

int handle_file(char* path)
{

}

void pr_usage()
{
    printf("Usage: \nmy_ping <ipaddress> [<ip_address>] - ping all listed ip addresses\n");
    printf("my_ping -r <ip-address> <ip-address> ping all ip addresses from selected range\n");
    printf("my_ping -f <path> - ping addresses from given file, addresses should be separated by a newline\n");
}

int main(int argc, char **argv)
{
    if(argc < 2){
        pr_usage();
        exit(-1);
    }
    // check if given ip is valid
    if(strcmp(argv[1],"-r")==0){
        // handle_range();
        if(argc > 3){
            fprintf(stderr,"Incorrect number of arguments: %d\n", argc);
            pr_usage();
            return -1
        }
        printf("To be implemented\n");
        return 0;
    }

    if(strcmp(argv[1],"-f")==0){
        // handle_file();
        printf("To be implemented\n");
        return 0;
    }

    handle_list(argc-1, argv);
    return 0;
}
