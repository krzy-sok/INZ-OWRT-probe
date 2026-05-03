#ifndef PINGROW
#define PINGROW
#include <string>
#include <vector>

class PingRow
{
    private:
        double _rtt;
        int _timestamp;

    public:
        PingRow(double rtt, int timestamp);
        std::string to_string();
        std::string to_json();
};
#endif