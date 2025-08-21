#ifndef HOST
#define HOSt
#include <string>
#include "ping_row.hpp"

class Host
{
    private:
        std::string ip;
        std::string mac;
        std::string interface;

    public:
        Host(std::string ip, std::string mac, std::string interface);
        PingRow ping();
};
#endif