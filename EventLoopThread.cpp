#include"EventLoopThread.hpp"

#include"EventLoop.hpp"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
        const std::string &name)
        :loop_(nullptr)
        ,exiting_(false)
        ,thread_(std::bind(&EventLoopThread::threadFunc,this),name)//创建一个thread对象，但还没有真正创建线程，要等start后
        ,mutex_()//默认构造
        ,cond_()//默认构造
        ,callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_=true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    //启动新线程，执行EventLoopThread::threadFunc
    thread_.start();

    EventLoop *loop=nullptr;
    //只有确保loop_已经指向了已创建的EventLoop后才能访问
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_==nullptr)
        {
            cond_.wait(lock);
        }
        loop=loop_;
    }
    return loop;
}

//下面这个方法是在单独的新线程中运行的
void EventLoopThread::threadFunc()
{
    //创建一个独立的eventLoop，和上面的线程是一一对应的 one loop per thread
    EventLoop loop;

    if(callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_one();
    }
    loop.loop();//开启Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
}
