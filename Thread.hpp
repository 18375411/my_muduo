#pragma once

#include"noncopyable.hpp"

#include<functional>
#include<thread>
#include<memory>
#include<unistd.h>
#include<string>
#include<atomic>

class Thread:noncopybale
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc,const std::string &name=std::string());
    ~Thread();

    void start();
    void join();

    bool started() const{return started_;}

    /**
     * pthread_self 是posix描述的线程ID（并非内核真正的线程id），
     * 相对于进程中各个线程之间的标识号，对于这个进程内是唯一的，
     * 而不同进程中，每个线程的 pthread_self() 可能返回是一样的。
     * 而 gettid 获取的才是内核中线程ID
    */
    pid_t tid() const{return tid_;}

    const std::string& name() const{return name_;}

    static int numCreated(){return numCreated_;}
private:

    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;

    static std::atomic_int numCreated_;
};