#include"EpollPoller.hpp"
#include"logger.hpp"
#include"Channel.hpp"
#include"Timestamp.hpp"

#include<errno.h>
#include<unistd.h>
#include<string.h>

const int kNew=-1;//Channel还未添加到Poller  channel的成员index初始化：-1
const int kAdded=1;//已添加
const int kDeleted=2;//已删除

EpollPoller::EpollPoller(EventLoop *loop)
    :Poller(loop)
    ,epoll_fd(::epoll_create1(EPOLL_CLOEXEC))//EPOLL_CLOEXEC作用：子进程在执行exec的时候，该文件描述符关闭
    ,events_(kInitEventListSize)
{
    if(epoll_fd<0)
    {
        LOG_FATAL("epoll_create error:%d\n",errno);
    }
}
EpollPoller::~EpollPoller()
{
    ::close(epoll_fd);
}

/**
 * Channel->update() remove() =======> EventLoop =======> Poller ========>EpollPoller
 *             EventLoop
 *   ChannelList           Poller
 *                     ChannelMap <fd,Channel*>
 *   所有Channel         该Poller中的Channel
*/
//用index来区分对Channel的操作，调用update()
void EpollPoller::updateChannel(Channel* channel)
{
    const int index=channel->index();
    LOG_INFO("func=%s => fd:%d events:%d index:%d",__FUNCTION__,channel->fd(),channel->events(),channel->index());

    if(index==kNew||index==kDeleted)
    {
        if(index==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }
        
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else//channel已经在poller上注册过了
    {
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }

}
//从Poller中移除Channel
void EpollPoller::removeChannel(Channel *channel)
{
    LOG_INFO("func=%s => fd:%d ",__FUNCTION__,channel->fd());

    int fd=channel->fd();
    channels_.erase(fd);

    if(channel->index()==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }

    channel->set_index(kNew);
}

//epoll_wait()  把发生事件的channel通过activeChannels告知给EventLoop
Timestamp EpollPoller::poll(int timeoutMs,ChannelList *activeChannels)
{
    //实际上应该用LOG_DEBUG更为合理
    LOG_INFO("func:%s => total_count:%d",__FUNCTION__,static_cast<int>(activeChannels->size()));

    int numberEvents=::epoll_wait(epoll_fd,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);

    int saveErrno=errno;//多线程中可能不同线程都会出错
    Timestamp now(Timestamp::now());

    if(numberEvents>0)
    {
        LOG_INFO("%d events happenned\n",numberEvents);
        fillActiveChannels(numberEvents,activeChannels);

        //扩容
        if(numberEvents==events_.size())//muduo库采用LT模式，未被处理完的事件下一次还会被触发
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numberEvents==0)
    {
        LOG_DEBUG("%s timeout\n",__FUNCTION__);
    }
    else
    {
        if(saveErrno!=EINTR)//EINTER外部中断
        {
            errno=saveErrno;
            LOG_ERROR("epoll_wait err:%d\n",errno);
        }
    }
    return now;

}

void EpollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const
{
    for(int i=0;i<numEvents;i++)
    {
        Channel* channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
        //EventLoop拿到了它的Poller给它返回的所有发生事件的channel
    }
}
//epoll_ctl add/mod/del
void EpollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    memset(&event,0,sizeof(event));
    event.events=channel->events();
    event.data.ptr=channel;

    int fd=channel->fd();

    if(::epoll_ctl(epoll_fd,operation,fd,&event)<0)
    {
        if(operation==EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n",errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
        }
    }

}