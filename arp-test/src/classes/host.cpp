#include <sys/socket.h>
#include <iostream>
#include <ctime>
#include <optional>

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
std::optional<PingRow> Host::ping(int sock)
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
        return {};
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

    clock_gettime(0, &t_sent);

    printf("socket desc: %d", sock);
    printf("packet type %d, packet code %d\n", packet->hdr.type, packet->hdr.code);
    int sent_res = sendto(sock, &packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr));
    if (sent_res <= 0){
        printf("Failed to send packet! %d\n", sent_res);
        printf("socaddr: %d\n\n", dst.sin_addr.s_addr);
        printf("soc sin family: %d\n\n", dst.sin_family);

        return {};
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
        return {};
    }

    clock_gettime(0, &t_recived);
    double rtt = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;

    struct icmphdr *recv_hdr = (struct icmphdr *)reply_buffer;
    // there is a bug in the kernel with how icmp packets are casted
    // https://blog.benjojo.co.uk/post/linux-icmp-type-69
    // meaning tahat i need a work around to check type and code as these values are unreliable
    // or skip this step of validation, sic!
    if (recv_hdr->type != 0 || recv_hdr->code != 0){
        std::cout<< "failed to recive.\ncode: " <<int(recv_hdr->code) << " type: " << int(recv_hdr->type) << std::endl;
        return {};
    }
    if(recv_hdr)
    printf("%d bytes from (ip: %s) rtt = %f ms.\n", PING_PKT_S, ip.data(), rtt);

    std::cout << rtt << std::endl;
    PingRow ping_res = PingRow(ip, mac, interface, rtt, std::time(nullptr));
    return ping_res;
}
