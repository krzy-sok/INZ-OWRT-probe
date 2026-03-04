#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include <netdb.h>

#include "host_file_reader.hpp"
#include "classes/host.hpp"
#include "classes/helpers/ip_range.cpp"


#define ARP_PATH       "/proc/net/arp"



std::optional<ipRange> hostFileReader::try_parse_ip_range(std::string range){
    std::stringstream ss(range);
    std::string first, last;

    getline(ss, first, '-');
    getline(ss, last, '-');

    if(first==""|last==""){
        return {};
    }

    in_addr start, end;
    if(inet_aton(first.data(), &start) == 0){
        return {};
    }
    if(inet_aton(last.data(), &end) == 0){
        return {};
    }

    ipRange rangeObj = ipRange(start.s_addr, end.s_addr);
    return rangeObj;
}

hostFileReader::hostFileReader(std::string filepath){
    _filepath = filepath;
}

void hostFileReader::add_host(std::stringstream &line_stream)
{
    std::string ip;
    std::string mac;
    std::string interface;
    line_stream >> ip;
    line_stream >> mac;
    line_stream >> interface;

    add_single_host(ip, mac, interface) || add_host_range(ip, mac, interface) || add_host_wildcard(ip, mac, interface);
    return;
}

void hostFileReader::exclude_host(std::stringstream &line_stream)
{
    // std::string ip;
    // std::string mac;
    // std::string interface;
    // line_stream >> ip;
    // line_stream >> mac;
    // line_stream >> interface;

    // exclude_single_host(ip, mac, interface) || exclude_host_range(ip, mac, interface) || add_host_wildcard(ip, mac, interface);
    return;
}

bool hostFileReader::add_single_host(std::string ip, std::string mac, std::string interface){
    in_addr addr;
    if(inet_aton(ip.data(), &addr) == 0){
        return false;
    }

    include_hosts.push_back({ip, mac, interface});

    return true;
}

bool hostFileReader::add_host_range(std::string ip, std::string mac, std::string interface)
{
    std::optional<ipRange> range_res = try_parse_ip_range(ip);
    if(!range_res.has_value()){
        return false;
    }
    ipRange range = range_res.value();

    std::string curr_addr = range.next_address();
    while(curr_addr !=""){
        // include_hosts.push_back(new Host(curr_addr, mac, interface));
        include_hosts.push_back(std::array<std::string, 3>{{curr_addr, mac, interface}});
        curr_addr = range.next_address();
    }
    return true;
}

bool hostFileReader::add_host_wildcard(std::string ip, std::string mac, std::string interface)
{
    return false;
}

void hostFileReader::parse_host_line(std::stringstream &line_stream){
    std::string action;
    line_stream >> action;

    if(action == "include"){
        add_host(line_stream);
    }
    else if(action == "exclude"){
        exclude_host(line_stream);
    }
}




// void parse_arp_table(std::stringstream &line_stream, std::vector<Host> &hosts, std::vector<std::string> exclude){
//     // arp format:
//     // IP address  HW type  Flags  HW address  Mask  Device
//     std::string ip;
//     std::string mac;
//     std::string interface;
//     std::string _;
//     line_stream >> ip;
//     line_stream >> _;
//     line_stream >> _;
//     line_stream >> mac;
//     line_stream >> _;
//     line_stream >> interface;
//     hosts.push_back(Host(ip, mac, interface));
// }

// // custom host file should not have to include all data from arp-table as haf of it is unused
// void parse_host_file(std::stringstream &line_stream, std::vector<Host> &hosts, std::vector<std::string> exclude){
//     std::string ip;
//     std::string mac;
//     std::string interface;
//     line_stream >> ip;
//     line_stream >> mac;
//     line_stream >> interface;
//     hosts.push_back(Host(ip, mac, interface));
// }

// std::vector<Host> read_host_file(std::string file_path){
//     std::vector<Host> hosts = {};
//     std::ifstream arp_cache(file_path);
//     std::string line;


//     std::function parser_func = parse_host_file;
//     if(file_path == ARP_PATH){
//         // skip file header
//         if(!std::getline(arp_cache, line))
//         {
//             std::cerr<<"Failed to read arp cache!\n";
//             throw std::runtime_error("filed to read arp cache");
//         }
//         parser_func = parse_arp_table;
//     }

//     while(std::getline(arp_cache, line))
//     {
//         std::stringstream line_stream(line);
//         parser_func(line_stream, hosts, );
//     }
//     return hosts;
// }