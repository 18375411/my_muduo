#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include"Timestamp.hpp"

#include<iostream>

TEST_CASE("Timestamp","[test]"){
    std::cout<<Timestamp::now().toString();
}