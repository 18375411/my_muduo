#include"Poller.hpp"
#include"EpollPoller.hpp"

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))//获取环境变量的值
    {
        return nullptr;
    }
    else
    {
        return new EpollPoller(loop);
    }
}
 