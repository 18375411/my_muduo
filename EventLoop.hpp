#pragma once

#include"noncopyable.hpp"
#include"Timestamp.hpp"
#include"CurrentThread.hpp"

#include<functional>
#include<vector>
#include<atomic>
#include<memory>
#include<mutex>


class Channel;
class Poller;


//事件循环类 主要包含两大模块 Channel Poller
class EventLoop:noncopybale
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();//开启事件循环

    void quit();//退出事件循环
    
    Timestamp pollReturnTime() const {return pollReturnTime_;}

    void runInLoop(Functor cb);//在当前loop执行cb
    void queueInLoop(Functor cb);//把cb放入队列中，唤醒loop所在线程，执行cb

    void wakeup();//唤醒loop所在线程

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const{ return threadId_==CurrentThead::tid();}//当前线程是否在创建自己的线程中


private:
    void handleRead();//wake up
    void doPendingFunctors();//执行回调


    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;//标志退出loop循环

    const pid_t threadId_;//记录当前loop所在线程的id

    Timestamp pollReturnTime_;//poller返回发生事件channel的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;//由eventfd()创建，当mainLoop获取一个新用户的channel，通过轮询算法选择一个subLoop，通过该成员唤醒subLoop处理Channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFuntors_;//标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;//存储Loop需要执行的所有回调操作
    std::mutex mutex_;//保护 std::vector<Functor>的线程安全操作
};