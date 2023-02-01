#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include"InetAdress.hpp"

#include<iostream>

TEST_CASE("InetAdress","[test]"){
    InetAdress addr(8080);
    std::cout<<addr.toIpPort()<<std::endl;
}