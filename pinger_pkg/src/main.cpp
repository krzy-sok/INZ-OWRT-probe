#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <random>
#include <iomanip>

#include "classes/host.hpp"
#include "classes/ping_row.hpp"

#define ARP_PATH       "/proc/net/arp"
#define DEFAULT_NPING_PARAMS "-udp --dest-mac 90-E8-68-14-94-44"



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

std::vector<Host> read_host_file(std::string file_path)
{
    std::vector<Host> hosts = {};
    std::ifstream arp_cache(file_path);
    std::string line;
    // skip file header
    if(!std::getline(arp_cache, line))
    {
        std::cerr<<"Failed to read arp cache!\n";
        throw std::runtime_error("filed to read arp cache");
    }

    std::function parser_func = parse_arp_table;
    if(file_path != ARP_PATH){
        parser_func = parse_host_file
    }

    while(std::getline(arp_cache, line))
    {
        std::stringstream line_stream(line);
        parser_func(line_stream, hosts);
    }
    return hosts;
}

void parse_arp_table(std::stringstream &line_stream, std::vector<Host> &hosts){
    // arp format:
    // IP address  HW type  Flags  HW address  Mask  Device
    std::string ip;
    std::string mac;
    std::string interface;
    std::string _;
    line_stream >> ip;
    line_stream >> _;
    line_stream >> _;
    line_stream >> mac;
    line_stream >> _;
    line_stream >> interface;
    hosts.push_back(Host(ip, mac, interface));
}

// custom host file should not have to include all data from arp-table as haf of it is unused
void parse_host_file(std::stringstream &line_stream, std::vector<Host> &hosts){
    std::string ip;
    std::string mac;
    std::string interface;
    line_stream >> ip;
    line_stream >> mac;
    line_stream >> interface;
    hosts.push_back(Host(ip, mac, interface));
}

std::string generate_mac_address() {
    // XX-XX-XX-XX-XX-XX
    std::random_device rng;
    std::ostringstream str_stream;
    std::uniform_int_distribution<int> dist(0,256);
    // exclude multicast and reserved
    int unicast_bitmask = 0b11111010;
    // exclude globally unique addresses
    int locally_unique_bitmask = 0b00000010;
    int first_octet = dist(rng) & unicast_bitmask | locally_unique_bitmask;
    str_stream <<  std::setw(2) << std::setfill('0') << std::hex << first_octet;
    for(int i = 1; i<=6; i++){
        str_stream <<":" <<  std::setw(2) << std::setfill('0') << std::hex << dist(rng);
    }
    return str_stream.str();
}

// target input params:
    // flood flag,
    // input file in arp-table format? or ip, mac, interface,
    // number of pings to each host,
    // nping options (as 1 string?) - default set in monitoring-scripts
int main(int argc, char* argv[])
{
    if(argc!=5){
        return -1;
    }
    std::string flood_flag = std::string(argv[1]);
    bool is_flood = flood_flag == "1";

    std::string file_path = argv[2];
    if(!std::filesystem::exists(file_path)){
        std::clog<< "Path: " << file_path << " does not exist!";
        return -1;
    }

    int ping_count = atoi(argv[3]);
    if(ping_count <= 0 || ping_count > 8){
        std::clog<<"Ping count has invalid value: " << ping_count << ". Ping count must have values between 1 and 8";
        return -1;
    }

    std::string nping_opts = std::string(argv[4]);

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        std::cerr << "Error crating socket\n";
        exit(-2);
    }
    std::clog<<"created socket\n";

    std::vector<Host> hosts = read_host_file();
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