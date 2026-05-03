#ifndef IPWILDCARD
#define IPWILDCARD

#include <string>
#include <array>
#include <vector>

#include "./wildcard_position.cpp"

class ipWildcard
{
    private:
        bool is_last_value;
        std::vector<int> increment_cnt;
        unsigned short curr_octets[4];
        std::vector<struct wildcard_position> wildcard_indexes;

        void increment_wildcard(int wildcard_index);
    public:
        ipWildcard(std::string ip);
        std::string next_address();
};

#endif