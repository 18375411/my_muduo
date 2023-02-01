#include"Poller.hpp"

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    /**
     * @todo
    */
    if(::getenv("MUDUO_USE_POLL"))//获取环境变量的值
    {
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}
 