// test to ping a machine of ip given on program call
//  to see how to ping, extract info and what will happen if adress is not valid

// references
// original ping.c
// https://github.com/amitsaha/ping/blob/master/ping.c
// same as before but with syntax highlight https://gist.github.com/bugparty/ccba5744ba8f1cece5e0
//
// simpler ping implementation
// https://nsrc.org/wrc/materials/src/ping.c


#include <stdio.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

#include <arpa/inet.h>


#define MAX_PACKET  1024
#define ODDBYTE(v) htons((unsigned short)(v) <<8)

struct timezone tz;

// //  ping a given ip
// // will send a few packets ~4
// // will get ip from rotate_target
// int ping_host()
// {


// }

// // host discovery
// // get ip and mac addrsess of devices connected to a given network
// // device running this program must be connected to the given network
// int host_discovery()
// {

// }

// // ping hosts cyclicly
// // periods should be configurable
// // rotation alg should be configurable
// // rotation algs will be in superate functions
// int rotate_target()
// {

// }

// // introduce dalays between ping periods
// int delay()
// {

// }


// // send collected data to openwisp-api
// // will be extracted to different file
// int send_to_api()
// {

// }

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

// calc internet checksum
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

// create socket
// get network ip from user
// will be extracted different file
int main(int argc, char **argv)
{
    int sock;
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

    // crate icmp socket
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    printf("created socket\n");

    // construct icmp packet, same way as in ping.c
    // reserve memory for packet
    static unsigned char packet[MAX_PACKET];

    // create icmp packet struct
    struct icmp *icmp_p = (struct icmp *) packet;

    int i,sent_len;
    int datalen = 56;

    // fill in packet info
    icmp_p->icmp_type = ICMP_ECHO;
    icmp_p->icmp_code = 0;
    icmp_p->icmp_cksum=0;

    // put timeval in icmp packet
    struct timeval *tp = (struct timeval *) &packet[8];
    unsigned char *datap = &packet[8+sizeof(struct timeval)];

    // get time for rtt
    gettimeofday(tp, &tz);

    // skip 8 bytes - icmp header
    for(i=8; i<datalen;i++){
        *datap++ = i;
    }


    // calculate checksum
    sent_len = datalen +8;
    icmp_p->icmp_cksum = in_cksum((unsigned short *)icmp_p, sent_len, 0);
    printf("checksum sent = %d\n", icmp_p->icmp_cksum);


    // send packet
    dump(packet, 64);
    i = sendto(sock, packet, MAX_PACKET, 0, (struct sockaddr*)&dst, sizeof(dst));


    // create structs and buffer to hold reply
    unsigned char buff[MAX_PACKET];
    struct icmp *icmp_reply;
    struct timeval *tv_recv;
    struct timeval tv_curr;
    int triptime;


    socklen_t dst_len = sizeof(dst);
    // recvmsg caused issues with icmp type
    int recv_len = recvfrom(sock, &buff, MAX_PACKET, 0, (struct sockaddr *)&dst, &dst_len );


    printf("len received = %d\n", recv_len);
    if (recv_len < 0){
        perror("Error in recvmsg");
        exit(1);
    }

    // dump(&buff, recv_len);

    // trip time calculation
    icmp_reply = (struct icmp *)(buff+20);
    gettimeofday(&tv_curr,&tz);
    tv_recv = (struct timeval *)&icmp_reply->icmp_data[0];

    printf("len of buff %ld\n", sizeof(buff));
    printf("checksum from icmp_reply = %d\n", icmp_reply->icmp_cksum);

    // TO READ ON:
    /* Note that we don't have to check the reply ID to match that whether
     * the reply is for us or not, since we are using IPPROTO_ICMP.
     * See https://lwn.net/Articles/443051/ ping_v4_lookup()
     * If we were using a RAW socket, we would need to do that.
     * */

    // address of our correspondent
    // struct sockaddr_in *from = msg.msg_name;

    // printf("ICMP code: %d\n", icmp_reply->icmp_code);
    // printf("ICMP type %d\n", icmp_reply->icmp_type);
    if(icmp_reply->icmp_type == ICMP_ECHOREPLY){
        // print ping reply
        printf("ICMP code: %d\n", icmp_reply->icmp_code);
        printf("ICMP type %d\n", icmp_reply->icmp_type);
        printf("rply of %d bytes recieved\n", recv_len);
        printf("icmp sequence: %u\n", ntohs(icmp_reply->icmp_seq));
    }

    close(sock);
    printf("closed socket\n");
    return 0;
}