#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <netinet/ip.h>

#define ODDBYTE(v) htons((unsigned short)(v) <<8)
#define PING_PKT_S 64

// since icmphdr does not include data
// and icmp struct does not send on modern system
struct icmp_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

// since recv from on raw socket is bugged and reads IP header
// we need to facilitate the ipheader
struct icmp_reply
{
	struct iphdr ip_hdr;
    struct icmphdr icmp_hdr;
};

// Calculate the checksum (RFC 1071)
unsigned short in_cksum(const unsigned short *addr, int len, unsigned short csum)
{
	int nleft = len;
	const unsigned short *w = addr;
	unsigned short answer;
	int sum = csum;

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