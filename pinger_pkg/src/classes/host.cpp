#include <sys/socket.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>
#include <optional>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "host.hpp"
#include "../ping_helpers.hpp"

Host::Host(std::string ip, std::string mac, std::string interface)
{
    _ip = ip;
    _mac = mac;
    _interface = interface;
    _pid = -1;
    if(inet_aton(ip.data(), &dst.sin_addr) == 0)
    {
        // TODO: dont throw - make the program able to go on or exit
        throw std::invalid_argument("Incorrect ipv4 address: " + _ip);
    }
    dst.sin_family = AF_INET;
}
// TODO: modernize to c++ where possible
std::optional<PingRow> Host::ping(int sock)
{
    struct timespec t_sent, t_recived;

    unsigned char packet_buffer[PING_PKT_S];
    struct icmp_pkt *packet = (struct icmp_pkt *) packet_buffer;

    // compose packet
    // set header
    packet->hdr.type = ICMP_ECHO;
    packet->hdr.code = 0;
    packet->hdr.checksum = 0;
    packet->hdr.un.echo.id = getpid();

    // set checksum
    packet->hdr.checksum = in_cksum((unsigned short *)packet, sizeof(packet_buffer), 0 );

    clock_gettime(0, &t_sent);

    std::clog<<"packet type "<<packet->hdr.type<<" packet code "<< packet->hdr.code << std::endl;
    int sent_res = sendto(sock, &packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr));
    if (sent_res <= 0){
        std::cerr << "Failed to send packet! "<< sent_res <<std::endl;
        std::cerr<<"socaddr: " << dst.sin_addr.s_addr <<std::endl;
        std::cerr<<"soc sin family: " << dst.sin_family <<std::endl;

        return {};
    }

    // prepare buffer
    unsigned char reply_buffer[128];
    socklen_t dst_len = sizeof(struct sockaddr);
    // receive reply
    int recv_res = recvfrom(sock, &reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&dst, &dst_len);
    if (recv_res <0){
        std::cerr<<"Failed to receive packet! "<< recv_res<< std::endl;
        return {};
    }

    clock_gettime(0, &t_recived);
    double rtt = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;

    struct icmp_reply *recv_hdr = (struct icmp_reply *)reply_buffer;
    // there is a bug in the kernel with how icmp packets are casted
    // https://blog.benjojo.co.uk/post/linux-icmp-type-69
    // meaning tahat i need a work around to check type and code as these values are unreliable
    // or skip this step of validation, sic!
    if (recv_hdr->icmp_hdr.type != 0 || recv_hdr->icmp_hdr.code != 0){
        std::clog<< "Failed to recive.\ncode: " <<int(recv_hdr->icmp_hdr.code) << " type: " << int(recv_hdr->icmp_hdr.type) << std::endl;
        return {};
    }
    if(recv_hdr)
    std::clog << PING_PKT_S <<" bytes from "<< _ip.data() << " rtt = " << rtt << " ms." << std::endl;

    PingRow ping_res = PingRow(_ip, _mac, _interface, rtt, std::time(nullptr));
    return ping_res;
}

std::vector<std::string> Host::split_args(std::string nping_args){
    std::stringstream ss(nping_args);

    std::string token;
    std::vector<std::string> argv;

    while (getline(ss, token, ' ')){
        argv.push_back(token);
    }
    return argv;
}

int Host::startFlood(std::string nping_args){
    int pid = fork();
    if(pid < 0){
        std::clog<<"could not fork process for flooding" <<std::endl;
        return -1;
    }

    if(pid==0){
        // abort child process on error
        int fd = open("/dev/null", O_WRONLY);
        if(fd<0){
            std::clog<< "Cannot open /dev/null to redirect nping output"<<std::endl;
            std::abort();
        }

        if(dup2(fd, 1) != 1){
            std::clog<< "Cannot redirect std::cout to /dev/null with dup2"<<std::endl;
            std::abort();
        }
        nping_args += " " + _ip;
        // std::clog<<std::endl<<"-----" << nping_args <<"-----"<< std::endl;
        std::vector<std::string> args = split_args(nping_args);
        std::vector<char *> argv;
        argv.push_back(const_cast<char *>("nping"));
        for (std::string& s : args) {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);

        if(execv("/usr/bin/nping", argv.data())<0){
            std::clog<< "Cannot call nping!"<<std::endl;
            std::cerr<<"execv errorno: "<< errno <<std::endl<< "error value"<< strerror(errno)<< std::endl;
            std::abort();
        }
        std::terminate();
    }
    else{
        _pid = pid;
    }
    return 1;
}

void Host::stopFlood(){
    if(_pid<=0){
        return;
    }
    int res = kill(_pid, SIGTERM);
    if(res<0){
        std::clog<<"Cannot terminate nping process with pid: "<<_pid <<std::endl;
        return;
    }
    _pid = -1;

}
