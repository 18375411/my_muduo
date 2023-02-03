#pragma once

#include"Poller.hpp"

#include<vector>
#include<sys/epoll.h>

class Channel;
class EventLoop;

/**
 * @attention 在私有继承和保护继承时基类指针(引用)无法指向派生类,因此EpollPoller需要public继承Poller
*/
class EpollPoller: public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;//让编译器检查一定是虚函数

    Timestamp poll(int timeoutMs,ChannelList *activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize=16;

    //填写活跃的链接
    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epoll_fd;
    EventList events_;
};