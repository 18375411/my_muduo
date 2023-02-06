#include"Channel.hpp"
#include"EventLoop.hpp"
#include"logger.hpp"

#include<sys/epoll.h>

const int Channel::kNoneEvent=0;
const int Channel::kReadEvent=EPOLLIN|EPOLLPRI;
const int Channel::kWriteEvent=EPOLLOUT;

Channel::Channel(EventLoop *loop,int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,index_(-1)//kNew
    ,tied_(false) 
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_=obj;
    tied_=true;
}
/**
 * 当该改变channel中fd的events事件时，update需要负责在poller里更改fd相应的事件epoll_ctl
*/
void Channel::update()
{
    //通过channel所属的EventLoop,调用poller相应的方法
   loop_->updateChannel(this);
}

void Channel::remove()
{
   loop_->removeChannel(this);
}

 void Channel::handleEvent(Timestamp receiveTime)
 {
    if(tied_){
        std::shared_ptr<void> guard=tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }
    else{
        handleEventWithGuard(receiveTime);//acceptor中的channel没有tie(因为acceptor不会给到用户手中，而TcpConnction会被暴露)
    }
 }

 void Channel::handleEventWithGuard(Timestamp receiveTime)
 {
    LOG_INFO("channel handleEvent revents:%d\n",revents_);
    
    if((revents_&EPOLLHUP) && !(revents_&EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if(revents_&(EPOLLIN|EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    if(revents_&EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
 }