#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include <cmath>
#include <netdb.h>

#include "host_file_reader.hpp"
#include "classes/host.hpp"
#include <regex>
#include "classes/helpers/ip_range.hpp"
#include "classes/helpers/ip_wildcard.hpp"


#define ARP_PATH       "/proc/net/arp"

void hostFileReader::parse_arp_line(std::stringstream &line_stream){
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

void hostFileReader::add_host(std::stringstream &line_stream,  std::vector<host_intermidiate> &target)
{
    std::string ip;
    std::string mac;
    std::string interface;
    line_stream >> ip;
    line_stream >> mac;
    line_stream >> interface;

    add_single_host(ip, mac, interface, target) || add_host_range(ip, mac, interface, target) || add_host_wildcard(ip, mac, interface, target);
    return;
}

bool hostFileReader::add_single_host(std::string ip, std::string mac, std::string interface,  std::vector<host_intermidiate> &target){
    in_addr addr;
    if(inet_aton(ip.data(), &addr) == 0){
        return false;
    }

    target.push_back({ip, mac, interface});

    return true;
}

bool hostFileReader::add_host_range(std::string ip, std::string mac, std::string interface,  std::vector<host_intermidiate> &target)
{
    std::optional<ipRange> range_res = try_parse_ip_range(ip);
    if(!range_res.has_value()){
        return false;
    }
    ipRange range = range_res.value();

    std::string curr_addr = range.next_address();
    while(curr_addr !=""){
        target.push_back({curr_addr, mac, interface});
        curr_addr = range.next_address();
    }
    return true;
}

bool hostFileReader::add_host_wildcard(std::string ip, std::string mac, std::string interface,  std::vector<host_intermidiate> &target)
{
    if(ip.find('*') == std::string::npos){
        return false;
    }
    std::string regex_octet = "((25([0-5]|\\*))|(2([0-4]|\\*)([0-9]|\\*))|(1([0-9]|\\*){2})|((0|\\*)?([0-9]|\\*){1,2}))";
    std::string regex_str = "(" + regex_octet + "\\.){3}" + regex_octet;
    std::regex ip_wildcard_reg(regex_str);

    if(!std::regex_match(ip, ip_wildcard_reg)){
        return false;
    }
    ipWildcard ip_wildcard = ipWildcard(ip);

    std::string res_addr = ip_wildcard.next_address();
    while(res_addr != ""){
        target.push_back({res_addr, mac, interface});
        res_addr = ip_wildcard.next_address();
    }
    return true;
}

void hostFileReader::parse_host_line(std::stringstream &line_stream){
    std::string action;
    line_stream >> action;

    if(action == "include"){
        add_host(line_stream, this->include_hosts);
    }
    else if(action == "exclude"){
        add_host(line_stream, this->exclude_hosts);
    }
}


std::vector<Host> hostFileReader::read_host_file(){
    std::vector<Host> hosts = {};
    std::ifstream arp_cache(_filepath);
    std::string line;

    // std::function<void(std::stringstream&)> parser_func;
    // parser_func =
    void (hostFileReader::*parser_func)(std::stringstream&) = nullptr;
    parser_func = &hostFileReader::parse_host_line;
    if(_filepath == ARP_PATH){
        // skip file header
        if(!std::getline(arp_cache, line))
        {
            std::cerr<<"Failed to read arp cache!\n";
            throw std::runtime_error("filed to read arp cache");
        }
        parser_func = &hostFileReader::parse_arp_line;
    }

    while(std::getline(arp_cache, line))
    {
        std::stringstream line_stream(line);
        parse_host_line(line_stream);
        // (this->*parser_func)(line_stream);
    }

    std::vector<host_intermidiate> diff;
    std::sort(include_hosts.begin(), include_hosts.end());
    std::sort(exclude_hosts.begin(), exclude_hosts.end());

    std::set_difference(include_hosts.begin(), include_hosts.end(),
        exclude_hosts.begin(), exclude_hosts.end(),
        std::back_inserter(diff));

    for(host_intermidiate entry : diff){
        hosts.push_back(Host(entry.ip, entry.mac, entry.interface));
    }
    return hosts;
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