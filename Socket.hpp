#pragma once

#include"noncopyable.hpp"

class InetAdress;

class Socket:noncopybale
{
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
        {}
    ~Socket();

    int fd() const{return sockfd_;}
    void bindAdress(const InetAdress &localAdress);
    void listen();
    int accept(InetAdress *peerAdress);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};