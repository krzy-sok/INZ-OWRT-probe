#ifndef IPRANGE
#define IPRANGE

#include <string>

class ipRange
{
    private:
        unsigned long octets_curr;
        unsigned long octets_end;

    public:
        ipRange(unsigned long start, unsigned long end);
        std::string next_address();
};

#endif