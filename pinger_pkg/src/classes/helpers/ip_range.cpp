#include "ip_range.hpp"

#include <sstream>
#include<string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ipRange::ipRange(unsigned long start, unsigned long end){
    octets_curr = start;
    octets_end = end;
}

std::string ipRange::next_address(){
    unsigned int max_value = htonl(4294967295);
    if(octets_curr > octets_end || octets_curr == max_value){
        return "";
    }

    struct in_addr curr_addr;
    curr_addr.s_addr = octets_curr;
    std::string str_adddr(inet_ntoa(curr_addr));

    unsigned long host_curr = ntohl(octets_curr);
    host_curr++;
    octets_curr = htonl(host_curr);

    return str_adddr;
}