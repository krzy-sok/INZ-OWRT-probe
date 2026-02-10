#include <sstream>
#include "ping_row.hpp"

PingRow::PingRow(std::string ip, std::string mac, std::string interface, double rtt, int timestamp, bool flood_flag)
{
    _ip = ip;
    _mac = mac;
    _interface = interface;
    _rtt = rtt;
    _timestamp = timestamp;
    _flood_flag = flood_flag;
}

std::string PingRow::to_string()
{
    std::ostringstream string_stream;
    string_stream << "ping result to IP: " << _ip << " rtt: " << _rtt << std::endl;
    return string_stream.str();
}

std::string PingRow::to_json()
{
    std::ostringstream string_stream;
    string_stream << "{ \"ip\":\"" << _ip
        <<"\", \"mac\":\"" << _mac
        << "\", \"rtt\": " << _rtt
        << ", \"timestamp\":" << _timestamp
        << ", \"flood_flag\": "
        << _flood_flag << ", \"interface\": \""
        << _interface
        <<"\"}";
    return string_stream.str();
}