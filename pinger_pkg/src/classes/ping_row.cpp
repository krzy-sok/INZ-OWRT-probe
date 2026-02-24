#include <sstream>
#include "ping_row.hpp"

PingRow::PingRow(double rtt, int timestamp)
{
    _rtt = rtt;
    _timestamp = timestamp;
}

std::string PingRow::to_string()
{
    std::ostringstream string_stream;
    string_stream << " rtt: " << _rtt << std::endl;
    return string_stream.str();
}

std::string PingRow::to_json()
{
    std::ostringstream string_stream;
    string_stream << "{ \"rtt\": " << _rtt
        << ", \"timestamp\":" << _timestamp
        <<"\"}";
    return string_stream.str();
}