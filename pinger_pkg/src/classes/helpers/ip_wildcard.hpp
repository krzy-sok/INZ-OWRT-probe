#ifndef IPWILDCARD
#define IPWILDCARD

#include <string>
#include <array>
#include <vector>

class ipWildcard
{
    private:
        bool is_last_value;
        unsigned short curr_octets[4];
        std::vector<std::array<int, 2>> wildcard_indexes;

        void increment_wildcard(int wildcard_index);
    public:
        ipWildcard(std::string ip);
        std::string next_address();
};

#endif