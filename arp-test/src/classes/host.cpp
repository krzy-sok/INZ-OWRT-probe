#include <sys/socket.h>
#include <iostream>
#include <ctime>

#include "host.hpp"
#include "../ping_helpers.hpp"

Host::Host(std::string ip, std::string mac, std::string interface)
{
    ip = ip;
    mac = mac;
    interface = interface;
    if(inet_aton(ip.data(), &dst.sin_addr) == 0)
    {
        // TODO: dont throw - make the program able to go on or exit
        throw std::invalid_argument("Incorrect ipv4 address: " + ip);
    }
    dst.sin_family = AF_INET;
}
// TODO: modernize to c++ where possible
PingRow Host::ping(int sock)
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
        // TODO - do something else than throw, tho it should not happen
        throw std::runtime_error("cannot set ttl");
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
        printf("soc sin family: %d\n\n", dst.sin_family);
        // dump(packet_buffer, PING_PKT_S);

        // TODO: dont throw, make program just continue
        throw std::runtime_error("Cannot send packet");
    }
    printf("socaddr: %d\n\n", dst.sin_addr.s_addr);
    printf("soc sin family: %d\n\n", dst.sin_family);

    // prepare buffer
    unsigned char reply_buffer[128];
    socklen_t dst_len = sizeof(struct sockaddr);
    // receive reply
    int recv_res = recvfrom(sock, &reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&dst, &dst_len);
    if (recv_res <0){
        printf("Failed to receive packet!%d\n", recv_res);
        // return -1;
        throw std::runtime_error("Cannot receive packet");
    }

    clock_gettime(0, &t_recived);
    double rtt = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;
    // double rtt = (t_recived.tv_sec - t_sent.tv_sec) * 1000 + time_elapsed;

    struct icmphdr *recv_hdr = (struct icmphdr *)reply_buffer;
    if (recv_hdr->type != 0 && recv_hdr->code!=0){
        printf("Not echo reply\n");
        throw std::runtime_error("Not an echo reply");
    }
    printf("%d bytes from (ip: %s) rtt = %f ms.\n", PING_PKT_S, ip.data(), rtt);

    std::cout << rtt << std::endl;
    PingRow ping_res = PingRow(ip, mac, interface, rtt, std::time(nullptr));
    return ping_res;
}
