#include <sstream>
#include <cmath>

#include "ip_wildcard.hpp"

ipWildcard::ipWildcard(std::string ip){
    std::stringstream ip_stream(ip);
    std::string str_octet;
    is_last_value = false;
    for(int i =0; i < 4; i++){
        getline(ip_stream, str_octet, '.');
        unsigned long int wildcard_ind = str_octet.find('*');
        while(wildcard_ind != std::string::npos){
            struct wildcard_position position;
            int padding = 3 - str_octet.size();
            position.octet =i;
            position.magnitude = std::pow(10, 2-(wildcard_ind+padding));

            wildcard_indexes.push_back(position);
            increment_cnt.push_back(0);

            str_octet.replace(wildcard_ind, 1, 1, '0');
            wildcard_ind = str_octet.find('*');
        }
        curr_octets[i] = atoi(str_octet.data());
    }
}

void ipWildcard::increment_wildcard(int wildcard_index){
    if(wildcard_index < 0){
        is_last_value = true;
        return;
    }
    int octet_index = wildcard_indexes[wildcard_index].octet;
    int increment = wildcard_indexes[wildcard_index].magnitude;

    if(increment_cnt[wildcard_index] == 9 ||curr_octets[octet_index] + increment > 255){
        int reset = curr_octets[octet_index] % (increment*10);
        curr_octets[octet_index] = curr_octets[octet_index] - reset;
        increment_cnt[wildcard_index] = 0;
        increment_wildcard(wildcard_index-1);
    }
    else{
        increment_cnt[wildcard_index]++;
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