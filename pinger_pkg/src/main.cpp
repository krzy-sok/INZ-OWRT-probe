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
        std::cerr<<"Failed to read arp cache!\n";
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

std::string combine_json(std::vector<PingRow> ping_results){
    if(ping_results.size() <1){
        return "";
    }
    std::ostringstream string_stream;
    string_stream << "\"probes\":[";
    string_stream << ping_results[0].to_json();
    for(long unsigned int i = 1; i<ping_results.size(); i++){
        string_stream << ", " << ping_results[i].to_json();
    }
    string_stream << "]";
    return string_stream.str();
}

// target input params: out folder path?, flood flag, input file in arp-table format
int main()
{
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        std::cerr << "Error crating socket\n";
        exit(-2);
    }
    std::clog<<"created socket\n";

    std::vector<Host> hosts = read_arp();
    std::vector<PingRow> ping_results = std::vector<PingRow>();
    for(Host host : hosts){
        std::optional<PingRow> ping_res = host.ping(sock);
        if(ping_res.has_value()){
            ping_results.push_back(ping_res.value());
        }
    }
    std::cout<<combine_json(ping_results);
    return 0;
}