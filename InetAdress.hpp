#pragma once

#include<netinet/in.h>
#include<string>

//封装Socket地址类型
class InetAdress{
public:
    explicit InetAdress(uint16_t port=0,std::string ip="127.0.0.1");
    explicit InetAdress(const sockaddr_in& addr)//const &既可以接受左值引用，也可以接受右值引用
        :addr_(addr)
    {}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const{ return &addr_;}

    void setSockAddr(const sockaddr_in &addr){addr_=addr;}
private:
    sockaddr_in addr_;
};