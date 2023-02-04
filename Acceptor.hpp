#pragma once

#include"noncopyable.hpp"
#include"Socket.hpp"
#include"Channel.hpp"

class EventLoop;
class InetAdress;
class Acceptor:noncopybale
{
public:
    using NewConnectionCallback = std::function<void(int sockfd,const InetAdress&)>;

    Acceptor(EventLoop* loop,const InetAdress &listenAddr,bool reusePort);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb){ newConnectionCallback_=std::move(cb);}

    bool listenning()const {return lisenning_;}
    void listen();

private:
    void handleRead();
    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool lisenning_;
};