#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>
#include <sys/time.h>
#include <algorithm>
#include <filesystem>
#include <random>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cmath>

#include "classes/host.hpp"
#include "classes/ping_row.hpp"
#include "host_file_reader.hpp"

#define ARP_PATH       "/proc/net/arp"
// #define DEFAULT_NPING_PARAMS "-udp --rate 1000 -c 2000 --dest-mac "
#define NPING_MODE "-tcp"
#define NPING_RATE 1000
#define TIMEOUT_SEC 4


std::string combine_json(std::vector<Host> hosts){
    if(hosts.size() <1){
        return "";
    }
    std::ostringstream string_stream;
    string_stream << "\"probes\":[";
    string_stream << hosts[0].to_json();
    for(long unsigned int i = 1; i<hosts.size(); i++){
        string_stream << ", " << hosts[i].to_json();
    }
    string_stream << "]";
    return string_stream.str();
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

std::vector<Host> read_host_file(std::string file_path)
{
    std::vector<Host> hosts = {};
    std::ifstream arp_cache(file_path);
    std::string line;


    std::function parser_func = parse_host_file;
    if(file_path == ARP_PATH){
        // skip file header
        if(!std::getline(arp_cache, line))
        {
            std::cerr<<"Failed to read arp cache!\n";
            throw std::runtime_error("filed to read arp cache");
        }
        parser_func = parse_arp_table;
    }

    while(std::getline(arp_cache, line))
    {
        std::stringstream line_stream(line);
        parser_func(line_stream, hosts);
    }
    return hosts;
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
    for(int i = 1; i<6; i++){
        str_stream <<":" <<  std::setw(2) << std::setfill('0') << std::hex << dist(rng);
    }
    return str_stream.str();
}

void set_socket_options(int sock){
    int ttl = 64;

    struct timeval tv_timeout;
    tv_timeout.tv_sec = TIMEOUT_SEC;
    tv_timeout.tv_usec = 0;

    // set ttl
    if (setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        std::cerr <<"Setting socket options to TTL failed!" << std::endl;
        throw std::runtime_error("filed tset socket options");
    } else {
        std::clog<<"Socket set to TTL..." << std::endl;
    }
    // set timeout on recive
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_timeout, sizeof tv_timeout);
}

// target input params:
    // flood flag,
    // input file in arp-table format? or ip, mac, interface,
    // number of pings to each host,
    // nping options (as 1 string?) - default set in monitoring-scripts
int main(int argc, char* argv[])
{
    if(argc< 4 || argc > 5){
        std::clog<< "Incorrect argument count! Required arguments:"<<std::endl<<"flood flag: 0/1 \n path to file with target hosts \n number of pings to each host \n optional nping options"<<std::endl;
        return -1;
    }
    std::string flood_flag = std::string(argv[1]);
    bool is_flood = flood_flag == "1";

    std::string file_path = argv[2];
    if(!std::filesystem::exists(file_path)){
        std::clog<< "Path: " << file_path << " does not exist!"<<std::endl;
        return -1;
    }

    int ping_count = atoi(argv[3]);
    if(ping_count <= 0 || ping_count > 8){
        std::clog<<"Ping count has invalid value: " << ping_count << ". Ping count must have values between 1 and 8" <<std::endl;
        return -1;
    }

    std::string nping_opts = "";
    if(argc == 5){
        nping_opts = std::string(argv[4]);
    }

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sock < 0){
        std::cerr << "Error crating socket\n"<<std::endl;
        exit(-2);
    }
    std::clog<<"created socket\n"<<std::endl;
    set_socket_options(sock);

    // std::vector<Host> hosts = read_host_file(file_path);
    hostFileReader reader = hostFileReader(file_path);
    std::vector<Host> hosts = reader.read_host_file();


    int delay = 1000;

    std::ostringstream nping_param;
    if(nping_opts != ""){
        nping_param << nping_opts;
    }
    else{
        nping_param << NPING_MODE
            << " --rate "<< NPING_RATE
            << " -c " << NPING_RATE * std::ceil(delay * ping_count /1000.0)
            << " --dest-mac " << generate_mac_address();
    }

    for(unsigned int i =0; i<hosts.size(); i++){
        if(is_flood){
            // std::cout<<"-----------------\n"<< "flood: " << is_flood<<std::endl;
            hosts[i].startFlood(nping_param.str());
        }

        std::chrono::milliseconds timespan(delay);
        for (int c=0; c<ping_count; c++){
            std::this_thread::sleep_for(timespan);
            hosts[i].ping(sock);

        }
        if(is_flood){
            hosts[i].stopFlood();
        }
    }
    std::cout<<combine_json(hosts)<<std::endl;
    return 0;
}