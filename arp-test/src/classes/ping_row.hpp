#ifndef PINGROW
#define PINGROW
#include <string>

class PingRow
{
    private:
        std::string ip;
        std::string mac;
        std::string interface;
        int rtt;
        int timestamp;

    public:
        PingRow(std::string ip, std::string mac, std::string interface, int rtt, int timestamp);
};
#endif