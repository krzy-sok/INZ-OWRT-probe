#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <algorithm>

#include "classes/host.hpp"
#include "classes/ping_row.hpp"

#define ARP_PATH       "/proc/net/arp"



void print_ping(Host host, int sock){
    auto pingres = host.ping(sock);
    if(pingres.has_value())
    {
        PingRow ping = pingres.value();
        std::cout<< ping.to_string();
    }
    else
    {
        std::cout << "Ping to host failed \n";
    }
}

std::vector<Host> read_arp()
{
    std::vector<Host> hosts = {};
    std::ifstream arp_cache(ARP_PATH);
    std::string line;
    // skip file header
    if(!std::getline(arp_cache, line))
    {
        std::cout<<"failed to read arp cache!";
        throw std::runtime_error("filed to read arp cache");
    }
    std::string ip;
    std::string mac;
    std::string interface;
    std::string _;
    while(std::getline(arp_cache, line))
    {
        std::stringstream line_stream(line);
        // arp format:
        // IP address  HW type  Flags  HW address  Mask  Device
        line_stream >> ip;
        line_stream >> _;
        line_stream >> _;
        line_stream >> mac;
        line_stream >> _;
        line_stream >> interface;
        hosts.push_back(Host(ip, mac, interface));
    }
    return hosts;
}

int main()
{
    // Host host = Host("8.8.8.8", "no", "no");

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        perror("Error crating socket\n");
        exit(-2);
    }
    std::cout<<"created socket\n";

    std::vector<Host> hosts = read_arp();
    for(Host host : hosts){
        print_ping(host, sock);
    }
    return 0;
}