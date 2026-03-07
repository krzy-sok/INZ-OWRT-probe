#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <iostream>
#include <fstream>


#define private public
#include "../src/classes/helpers/ip_wildcard.hpp"
#undef private

// TEST_CASE("sanity check"){
//     REQUIRE(false);
// }

TEST_CASE("construct wildcard1"){
    ipWildcard wildcard = ipWildcard("10.0.0.*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard2"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard3"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==10);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard4"){
    ipWildcard wildcard = ipWildcard("10.0.0.*00");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==100);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("construct wildcard5"){
    ipWildcard wildcard = ipWildcard("10.0.0.*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==10);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard2"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==1);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard3"){
    ipWildcard wildcard = ipWildcard("10.0.0.0*0");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==10);
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard4"){
    ipWildcard wildcard = ipWildcard("10.0.0.*00");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==100);


    std::string res = wildcard.next_address();

    REQUIRE(wildcard.is_last_value == false);
    REQUIRE(res == "10.0.0.0");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 100);
}

TEST_CASE("increment wildcard value edge"){
    ipWildcard wildcard = ipWildcard("10.0.0.*00");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==100);


    wildcard.next_address();
    std::string res = wildcard.next_address();

    REQUIRE(wildcard.is_last_value == false);
    REQUIRE(res == "10.0.0.100");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 200);

    res = wildcard.next_address();

    REQUIRE(wildcard.is_last_value == true);
    REQUIRE(res == "10.0.0.200");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);

    res = wildcard.next_address();

    REQUIRE(wildcard.is_last_value == true);
    REQUIRE(res == "");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 0);
    REQUIRE(wildcard.curr_octets[2] == 0);
    REQUIRE(wildcard.curr_octets[3] == 0);
}

TEST_CASE("increment wildcard value edge single star"){
    ipWildcard wildcard = ipWildcard("10.0.0.*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==1);


    wildcard.next_address();

    std::string res = wildcard.next_address();
    REQUIRE(res == "10.0.0.1");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.2");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.3");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.4");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.5");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.6");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.7");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.8");

    res = wildcard.next_address();
    REQUIRE(res == "10.0.0.9");

    res = wildcard.next_address();

    REQUIRE(res == "");
    REQUIRE(wildcard.is_last_value);

    res = wildcard.next_address();

    REQUIRE(res == "");
    REQUIRE(wildcard.is_last_value);
}

TEST_CASE("increment wildcard value edge single star loop"){
    ipWildcard wildcard = ipWildcard("10.0.0.*");
    REQUIRE(wildcard.wildcard_indexes.size()==1);
    REQUIRE(wildcard.wildcard_indexes[0].octet==3);
    REQUIRE(wildcard.wildcard_indexes[0].magnitude==1);

    std::string res = wildcard.next_address();
    int cnt = 0;
    while(res != ""){
        res = wildcard.next_address();
        cnt++;
    }
    REQUIRE(cnt==10);
}

TEST_CASE("construct with wildcards on all decials"){
    ipWildcard wildcard = ipWildcard("10.19*.1*1.*12");
    REQUIRE(wildcard.curr_octets[0] == 10);
    REQUIRE(wildcard.curr_octets[1] == 190);
    REQUIRE(wildcard.curr_octets[2] == 101);
    REQUIRE(wildcard.curr_octets[3] == 12);
}
