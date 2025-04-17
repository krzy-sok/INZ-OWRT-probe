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

unsigned short
in_cksum(const unsigned short *addr, register int len, unsigned short csum)
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
    // struct sockaddr_in source = {.sin_family = AF_INET};
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
    // assign a port
    // to be rethinked - port can be blocked and plan will go kaput
    // dst.sin_port = htons(1025);

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
    struct icmphdr *icmp_p = (struct icmphdr *) packet;

    int i,sent_len;
    int datalen = 56;
    // struct timezone tz;

    // fill in packet info
    // struct timeval *tp = (struct timeval *) &packet[8];
    // unsigned char *datap = &packet[8+sizeof(struct timeval)];
    icmp_p->type = ICMP_ECHO;
    icmp_p->code = 0;
    icmp_p->checksum=0;

    // get time for rtt
    // gettimeofday(tp, &tz);

    // calculate checksum
    sent_len = datalen +8;
    icmp_p->checksum = in_cksum((unsigned short *)icmp_p, sent_len, 0);
    printf("checksum sent = %d\n", icmp_p->checksum);

    // skip 8 bytes - idk why - is in ping.c
    // seems related to timeval
    // for(i=8; i<datalen;i++){
    //     *datap++ = i;
    // }

    // send packet
    i = sendto(sock, icmp_p, sent_len, 0, (struct sockaddr*)&dst, sizeof(dst));

    // try to read reply
    struct msghdr msg;
    int polling;
    char addrbuf[128];
    // io vector
    // the more i write in c the more i appreciate c++
    struct iovec iov;

    // turn addrbuff to vector
    iov.iov_base = addrbuf;
    // TODO: change to more space efficient version
    // that is - hard limit num of sent bytes to some small number
    // preferably just to transmit timeval
    iov.iov_len = MAX_PACKET;

    // clear memory taken by msg of garbage
    memset(&msg, 0 , sizeof(msg));

    msg.msg_name = addrbuf;
    msg.msg_namelen = sizeof(addrbuf);
    /* TODO: read this
     * https://www.safaribooksonline.com/library/view/linux-system-programming/9781449341527/ch04.html
     and here is msghdr source
     https://docs.huihoo.com/doxygen/linux/kernel/3.7/include_2linux_2socket_8h_source.html#l00050
     */

    // assign vector to message struct
    // msg.msg_iov = &iov;
    // ?? 1? why not iov.iov_len
    // to future me - it is len of array of vectors
    // msg.msg_iovlen = 1;

    // blocking wait on response
    // polling = MSG_WAITALL;
    // int recv_len = recvmsg(sock, &msg, polling);
    unsigned char buff[MAX_PACKET];

    // error - bad address
    socklen_t dst_len = sizeof(dst);
    int recv_len = recvfrom(sock, &buff, MAX_PACKET, 0, (struct sockaddr *)&dst, &dst_len );
    dump(&buff, recv_len);
    printf("len received = %d\n", recv_len);
    if (recv_len < 0){
        perror("Error in recvmsg");
        exit(1);
    }
    // in github/amitsaha/ping __u8 is used instead of uint8_t
    // but answer to this stack question says its too old and unportable to use: https://stackoverflow.com/questions/16232557/difference-between-u8-and-uint8-t
    // uint8_t * buf = msg.msg_iov->iov_base;
    struct icmphdr *icmp_reply;
    // int *skip_ip = *buff +20;
    icmp_reply = (struct icmphdr *)(buff);
    // printf("buffer = %s", buff);
    // icmp_reply = (struct icmphdr *)buff;

    // check checksum
    recv_len = recv_len ;
    printf("len of buff %ld\n", sizeof(buff));
    printf("checksum from icmp_reply = %d\n", icmp_reply->checksum);
    printf("icmp p sequence: %d\n", icmp_p->un.echo.sequence);

    // TO READ ON:
    /* Note that we don't have to check the reply ID to match that whether
     * the reply is for us or not, since we are using IPPROTO_ICMP.
     * See https://lwn.net/Articles/443051/ ping_v4_lookup()
     * If we were using a RAW socket, we would need to do that.
     * */

    // address of our correspondent
    // struct sockaddr_in *from = msg.msg_name;

    // calc triptime
    // to be studied
    // look at modern ping perhaps?
    // icmp_hdr does not have data field
    // will do this when ping works
    // or drop the idea entirely
    // tp = (struct timeval *)&icmp_reply->

    printf("ICMP code: %d\n", icmp_reply->code);
    printf("ICMP type %d\n", icmp_reply->type);
    if(icmp_reply->type == ICMP_ECHOREPLY){
        // print ping reply
        printf("ICMP code: %d\n", icmp_reply->code);
        printf("ICMP type %d\n", icmp_reply->type);
        printf("rply of %d bytes recieved\n", recv_len);
        printf("icmp sequence: %u\n", ntohs(icmp_reply->un.echo.sequence));
    }


    // TODO: napisać promotora jak własny musi być ping -
    //  czy mam sie męczyć z własną implementacja czy moge zmodyfikować oryginał czy mam użyć exec/skryptu

    close(sock);
    printf("closed socket\n");
    return 0;
}