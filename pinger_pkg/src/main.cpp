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
#include <cstring>

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

std::string generate_mac_address() {
    // XX-XX-XX-XX-XX-XX
    std::random_device rng;
    std::ostringstream str_stream;
    std::uniform_int_distribution<int> dist(0,256);
    // exclude multicast and reserved
    int unicast_bitmask = 0b11111010;
    // exclude globally unique addresses
    // int locally_unique_bitmask = 0b00000010;
    int first_octet = dist(rng) & unicast_bitmask;
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
    // int curl_flag
    // nping options (as 1 string?) - default set in monitoring-scripts
    // string path to file that should be gotten with curl
    // int curl port
int main(int argc, char* argv[])
{
    if(argc< 4 || argc > 8){
        std::clog<< "Incorrect argument count! Required arguments:"<<std::endl<<"flood flag: 0/1 \n path to file with target hosts \n number of pings to each host \n curlflag \n optional nping options \n curl path \ncurl port"<<std::endl;
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

    std::string curl_flag = std::string(argv[4]);
    bool is_curl = curl_flag == "1";
    std::clog<<is_curl;

    std::string nping_opts = "";
    if(argc >= 6 && strlen(argv[5]) != 0){
        nping_opts = std::string(argv[5]);
    }

    if(is_curl && argc<8){
        std::clog<<"to use curl you need to set path and port as 6 and 7 argument" <<std::endl;
        std::clog<<"reverting to ECHO ICMP method" <<std::endl;
        is_curl = false;
    }

    int curl_port;
    std::string curl_path;
    if(is_curl){
        std::clog<<"setting curl opts"<<std::endl;
        curl_path = std::string(argv[6]);
        curl_port = atoi(argv[7]);
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
            << " --rate "<< NPING_RATE;
    }
    nping_param << " -c " << NPING_RATE * std::ceil(delay * ping_count /1000.0)
         << " --dest-mac " << generate_mac_address();

    for(unsigned int i =0; i<hosts.size(); i++){
        if(is_flood){
            hosts[i].startFlood(nping_param.str());
        }

        std::chrono::milliseconds timespan(delay);
        for (int c=0; c<ping_count; c++){
            std::this_thread::sleep_for(timespan);
            if(is_curl){
                hosts[i].curl(curl_path, curl_port, TIMEOUT_SEC);
            }
            else{
                hosts[i].ping(sock);
            }
        }
        if(is_flood){
            hosts[i].stopFlood();
        }
    }
    std::cout<<combine_json(hosts)<<std::endl;
    return 0;
}