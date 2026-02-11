#include <catch2/catch_test_macros.hpp>


TEST_CASE("test pass"){
    REQUIRE(true);
}

TEST_CASE("test fail"){
    REQUIRE(false);
}