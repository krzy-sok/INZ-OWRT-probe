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
#include <cstring>
#include <curl/curl.h>

#include "host.hpp"
#include "../ping_helpers.hpp"

Host::Host(std::string ip, std::string mac, std::string interface)
{
    _ip = ip;
    _mac = mac;
    _interface = interface;
    _pid = -1;
    _flood_flag = 0;
    if(inet_aton(ip.data(), &dst.sin_addr) == 0)
    {
        // TODO: dont throw - make the program able to go on or exit
        throw std::invalid_argument("Incorrect ipv4 address: " + _ip);
    }
    dst.sin_family = AF_INET;
}
// TODO: modernize to c++ where possible
void Host::ping(int sock)
{
    struct timespec t_sent, t_recived;
    struct sockaddr_in src;

    unsigned char packet_buffer[PING_PKT_S] = {0};
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

    // std::clog<<"packet type: "<< packet->hdr.type <<"  packet code: " << packet->hdr.code << std::endl;
    int sent_res = sendto(sock, &packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&dst, sizeof(struct sockaddr));
    if (sent_res <= 0){
        std::cerr << "Failed to send packet! "<< sent_res <<std::endl;
        std::cerr<<"socaddr: " << dst.sin_addr.s_addr <<std::endl;
        std::cerr<<"soc sin family: " << dst.sin_family <<std::endl;

        return;
    }

    // prepare buffer
    unsigned char reply_buffer[128];
    memset(reply_buffer, 0, sizeof(reply_buffer));
    socklen_t src_len = sizeof(struct sockaddr);
    // receive reply
    int recv_res = recvfrom(sock, &reply_buffer, sizeof(reply_buffer), 0, (struct sockaddr *)&src, &src_len);
    if (recv_res <0){
        std::cerr<<"Failed to receive packet! "<< recv_res<< std::endl;
        return;
    }
    clock_gettime(0, &t_recived);
    double rtt = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;

    struct iphdr* ip_hdr = (struct iphdr *)reply_buffer;
    int ip_hdr_len = ip_hdr->ihl * 4;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(reply_buffer + ip_hdr_len);

    if(ip_hdr->protocol != IPPROTO_ICMP){
        std::clog<<"Incorrect response protocol! "<< ip_hdr->protocol <<std::endl;
        return;
    }

    if(src.sin_addr.s_addr != dst.sin_addr.s_addr){
        std::clog<<"Response has incorrect sender address! Received:"<< src.sin_addr.s_addr << ", expected:" << dst.sin_addr.s_addr <<std::endl;
        return;
    }

    if (icmp_hdr->type != ICMP_ECHOREPLY || icmp_hdr->code != 0){
        std::clog<< "Failed to recive.\ncode: " <<int(icmp_hdr->code) << " type: " << int(icmp_hdr->type) << std::endl;
        return;
    }

    if(icmp_hdr->un.echo.id != packet->hdr.un.echo.id)
    {
        std::clog<< "Wrong identifier.\ncode: " <<int(icmp_hdr->code) << " type: " << int(icmp_hdr->type) << std::endl
            << "daddr: " << ip_hdr->daddr << "proto:  " << ip_hdr->protocol << std::endl;
        std::clog << PING_PKT_S <<" bytes from "<< _ip.data() << " rtt = " << rtt << " ms." << std::endl;
        return;
    }
    // std::clog <<"recv.id: "<< icmp_hdr->un.echo.id << " sent.id: "<<packet->hdr.un.echo.id<<std::endl;

    PingRow ping_res = PingRow(rtt, std::time(nullptr));
    ping_results.push_back(ping_res);
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void Host::curl(std::string path, int port, long timeout){
    static const char *pagefilename = "/tmp/openwisp/curl-res";
    struct timespec t_sent, t_recived;
    CURLcode result;
    CURL *curl;

    result = curl_global_init(CURL_GLOBAL_ALL);
    if(result != CURLE_OK) {
        fprintf(stderr, "Could not init curl\n");
        return;
    }

    /* init the curl session */
    curl = curl_easy_init();
    if(curl) {
        FILE *pagefile;

        /* set URL to get here */
        std::stringstream curl_url;
        curl_url << _ip << ":" << port << path;
        std::clog<<"curl url: "<< curl_url.str() <<std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, curl_url.str().data());

        // /* Switch on full protocol/debug output while testing */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

        // // /* disable progress meter, set to 0L to enable it */
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

        // /* send all data to this function */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);


        /* open the file */
        pagefile = fopen(pagefilename, "wb");
        if(pagefile) {
            /* write the page body to this file handle */
            clock_gettime(0, &t_sent);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

            /* get it! */
            result = curl_easy_perform(curl);
            /* close the header file */
            fclose(pagefile);
            clock_gettime(0, &t_recived);
            if(!result){
                std::clog<<"cannot get document"<< path << "from hosts" << _ip;
            }
            else{
                double rtt = ((double)(t_recived.tv_nsec - t_sent.tv_nsec))/1000000;
                PingRow ping_res = PingRow(rtt, std::time(nullptr));
                ping_results.push_back(ping_res);
            }
        }
        /* cleanup curl stuff */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
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
    nping_args = nping_args + " " + _ip;
    std::clog<<"nping args: "<<nping_args <<std::endl;

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
        _flood_flag = 1;
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

std::string Host::to_json(){
    std::ostringstream string_stream;
    string_stream << "{ \"ip\": \""<< _ip <<"\""
        << ", \"mac\":\"" << _mac << "\""
        << ", \"flood_flag\": "<< _flood_flag
        << ", \"interface\":\""<< _interface << "\""
        << ", \"probes\": [";
    if (ping_results.size() >0){
        string_stream << ping_results[0].to_json();
    }
    for(long unsigned int i=1; i<ping_results.size(); i++){
        string_stream <<","<< ping_results[i].to_json();
    }
    string_stream << "]}";
    return string_stream.str();
}
