#include"EventLoop.hpp"
#include"logger.hpp"
#include"Poller.hpp"
#include"Channel.hpp"

#include<sys/eventfd.h>
#include<unistd.h>
#include<errno.h>



//防止一个线程创建多个EvenLoop thread_local
__thread EventLoop *t_loopInThisThread=nullptr;

//定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs=10000;

//创建wakeupfd，用来notify subreactor处理新来的channel
int createEventfd()
{
    int evtfd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evtfd<0)
    {
        LOG_FATAL("eventfd error:%d\n",errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    :looping_(false)
    ,quit_(false)
    ,callingPendingFuntors_(false)
    ,threadId_(CurrentThead::tid())
    ,poller_(Poller::newDefaultPoller(this))
    ,wakeupFd_(createEventfd())
    ,wakeupChannel_(new Channel(this,wakeupFd_))
{
    LOG_DEBUG("eventLoop created %p in thread %d",this,threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another Event12Loop %p exists in this thread %d\n",t_loopInThisThread,threadId_);
    }
    else
    {
        t_loopInThisThread=this;
    }

    //为什么ReadEventCallback与handleEvent参数不匹配？
    //===> The object produced by std::bind is designed to accept and ignore any extraneous parameters.
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));//类成员方法需要取地址
    /**
     * 每一个EventLoop都将监听wakeupChannel的可读事件
    */
    wakeupChannel_->enbleReading();

}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread=nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one=1;
    ssize_t n=read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERROR("%s reads %lu bytes instead of 8\n",__FUNCTION__,n);
    }
}

void EventLoop::loop()
{
    looping_=true;
    quit_=false;

    LOG_INFO("EventLoop %p start looping\n",this);

    while(!quit_)
    {
        activeChannels_.clear();
        //监听两类fd：一种是client的fd，一种wakeupFd
        pollReturnTime_=poller_->poll(kPollTimeMs,&activeChannels_);

        for(Channel* channel:activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作
        /**
         * mainLoop事先注册一个回调cb（需要subLoop)执行， wakeup subLoop后，subLoop执行下面的方法
        */
        doPendingFunctors();
    }
    LOG_INFO("Eventloop %p stop looping",this);
    looping_=false;
}

//退出事件循环： 1.loop在自己的线程中调用quit  2.在非loop的线程中，调用loop的quit
void EventLoop::quit()
{
    quit_=true;

    //如果是在其他的线程中，调用quit    如在一个subloop中，调用了mainloop的quit
    if(!isInLoopThread())
    {
        wakeup();//因为不知道要quit的线程的状态，是否处于阻塞之中，因此需要唤醒
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();//在当前线程调用runInLoop，说明已经未阻塞在poll中了
    }
    else//在非当前loop线程中执行cb,就需要唤醒该loop执行cb
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        //可能同时有多个线程向该loop传递cb，因此需要加锁
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    //唤醒相应的线程  1.在非当前loop线程中执行cb  2.上一轮的loop正在执行回调，为防止下一轮循环中阻塞在poll，需要wakeup
    if(!isInLoopThread()||callingPendingFuntors_)
    {
        wakeup();
    }
}

//用来唤醒loop所在的线程  向wakeupFd写一个数据，wakeupChannel就会发生可读事件，当前loop被唤醒
void EventLoop::wakeup()
{
    uint64_t one=1;
    ssize_t n=::write(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        LOG_ERROR("Eventloop::wakeup() writes %lu byetes instead of 8\n",n);
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
   return  poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    //注意这里先定义了一个局部vector，把pendingFunctors_的内容交换进去，再挨个处理，防止因执行回调造成pendingFunctors_阻塞时间过长
    std::vector<Functor> functors;
    callingPendingFuntors_=true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    // 使用const 修饰引用，使得只能只读，不能修改容器的数据
    for(const Functor &functor: functors)
    {
        functor();//执行当前loop需要执行的回调操作
    }
    callingPendingFuntors_=false;
}