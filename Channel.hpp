#pragma once

#include"noncopyable.hpp"
#include"Timestamp.hpp"

#include<functional>
#include<memory>
/**
 * 封装sock_fd及其感兴趣的event，如EPOLLIN事件等
 * 还绑定了poller返回的事件
*/

class EventLoop;
class Channel : noncopybale
{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;

    Channel(EventLoop *loop,int fd);
    ~Channel();

    //fd得到poller通知以后，处理事件的方法（调用回调方法）
    void handleEvent(Timestamp receiveTime);
    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){ readCallback_=std::move(cb);}
    void setWriteCallback(EventCallback cb){ writeCallback_=std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_=std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_=std::move(cb);}

    //被调用在TcpConnection::connectEstablished()中，这是由于channel中设置的回调都是TcpConnection中的成员
    //如果TcpConnection不存在了，再执行回调将会导致未知结果，因此需要weak_ptr监听TcpConnection的生存状态
    void tie(const std::shared_ptr<void> &);

    int fd() const{return fd_;}
    int events() const {return events_;}
    void set_revents(int rev){revents_=rev;}

    //设置fd相应的事件状态
    void enbleReading(){events_|=kReadEvent; update();}
    void disableReading(){events_&=~kReadEvent; update();}
    void enbleWriting(){events_|=kWriteEvent; update();}
    void disableWriting(){events_&=~kWriteEvent; update();}
    void disableAll(){events_=kNoneEvent; update();}

    //返回fd当前的事件状态
    bool isNoneEvent() const {return events_==kNoneEvent;}
    bool isReading() const {return events_ & kReadEvent;}
    bool isWring() const {return events_ & kWriteEvent;}

    int index() const {return index_;}
    void set_index(int idx){index_=idx;}

    EventLoop *ownerLoop(){return loop_;}//当前channel属于的EventLoop
    void remove();

private: 

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;//事件循环
    const int fd_;//Poller监听的对象
    int events_;//注册fd感兴趣的对象 epoll_ctl
    int revents_;//poller返回具体发生的事件
    int index_; //Channel在poller中的状态

    //跨线程生存状态监听
    std::weak_ptr<void> tie_;
    bool tied_;

    //由于Channel可以知道revents_，所以它负责调用具体事件的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

