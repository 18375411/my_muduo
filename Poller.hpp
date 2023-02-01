#pragma once

#include"noncopyable.hpp"
#include"Timestamp.hpp"

#include<vector>
#include<unordered_map>

class Channel;
class EventLoop;
/**
 * muduo库中多路事件分发器的核心IO复用模块
*/
class Poller: noncopybale
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller()=default;

    //给所有IO复用保留统一接口
    virtual Timestamp poll(int timeoutMs,ChannelList *activeChannels)=0;

    virtual void updateChannel(Channel* channel)=0;

    virtual void removeChannel(Channel *channel)=0;

    //判断channel是否在当前Poller中
    virtual bool hasChannel(Channel *channel) const;

    //EventLoop可以通过该接口获取默认的IO复用的具体实现(由于需要include派生类Poll/Epoll，因此不要在基类中实现)
    static Poller* newDefaultPoller(EventLoop *loop);
protected:
    //map的key：sock_fd  value: sockfd所属的Channel
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};