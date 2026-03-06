#include <sstream>
#include <cmath>

#include "ip_wildcard.hpp"

ipWildcard::ipWildcard(std::string ip){
    std::stringstream ip_stream(ip);
    std::string str_octet;
    is_last_value = false;
    for(int i =0; i < 4; i++){
        getline(ip_stream, str_octet, '.');
        int wildcard_ind = str_octet.find('*');
        if(wildcard_ind != std::string::npos){
            int padding = 3 - str_octet.size();
            wildcard_indexes.push_back(std::array<int, 2>{i,wildcard_ind+padding});
            // str_octet[wildcard_ind] = '0';
            str_octet.replace(wildcard_ind, 1, 1, '0');
        }
        curr_octets[i] = atoi(str_octet.data());
    }
}

void ipWildcard::increment_wildcard(int wildcard_index){
    if(wildcard_index<0){
        is_last_value = true;
        return;
    }
    int octet_index = wildcard_indexes[wildcard_index][0];
    int decimal_index = wildcard_indexes[wildcard_index][1];

    int increment = pow(10, 2-decimal_index);
    if(curr_octets[octet_index] + increment > 255){
        int reset = curr_octets[octet_index] % (increment*10);
        curr_octets[octet_index] = curr_octets[octet_index] - reset;
        increment_wildcard(wildcard_index-1);
    }
    else{
        curr_octets[octet_index] += increment;
    }
}

std::string ipWildcard::next_address(){
    if(is_last_value){
        return "";
    }
    std::stringstream ss;
    ss<<curr_octets[0];
    for(int i =1; i<4;i++){
        ss<<"."<<curr_octets[i];
    }
    int last_wildcard_index = wildcard_indexes.size()-1;
    increment_wildcard(last_wildcard_index);

    return ss.str();
}