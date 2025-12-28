#ifndef PINGROW
#define PINGROW
#include <string>

class PingRow
{
    private:
        std::string _ip;
        std::string _mac;
        std::string _interface;
        double _rtt;
        int _timestamp;

    public:
        PingRow(std::string ip, std::string mac, std::string interface, double rtt, int timestamp);
        std::string to_string();
        std::string to_json();
};
#endif