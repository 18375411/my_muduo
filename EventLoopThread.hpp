#pragma once

#include"noncopyable.hpp"
#include"Thread.hpp"

#include<functional>
#include<mutex>
#include<condition_variable>
#include<string>

class EventLoop;
/**
 * 封装了一个Thread和一个EventLoop 实现per thread per loop
*/
class EventLoopThread:noncopybale
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb =ThreadInitCallback(),
        const std::string &name=std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:

    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};