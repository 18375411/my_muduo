#pragma once

#include"noncopyable.hpp"

#include<functional>
#include<string>
#include<vector>
#include<memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool:noncopybale
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *baseLoop,const std::string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNumber(int numberThreads){numThreads_=numberThreads;}

    void start(const ThreadInitCallback &cb=ThreadInitCallback());

    //如果工作在多线程中，baseLoop默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops() const;

    bool started() const {return started_;}
    const std::string& name(){return name_;}
private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};