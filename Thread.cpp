#include"Thread.hpp"
#include"CurrentThread.hpp"

#include<semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func,const std::string &name)
    :started_(false)
    ,joined_(false)
    ,tid_(0)
    ,func_(std::move(func))
    ,name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        //内核资源在线程结束时自动回收
        thread_->detach();//thread类提供的分离线程方法
    }
}

void Thread::start()
{
    started_=true;

    sem_t sem;
    sem_init(&sem,false,0);

    //开启线程
    thread_=std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程tid
        tid_=CurrentThread::tid();

        sem_post(&sem);
        //新线程专门执行该线程函数
        func_();

    }));


    //由于无法确定线程执行的顺序，因此必须确认tid_已经获取成功才可以退出
    sem_wait(&sem);
}

void Thread::join()
{
    joined_=true;
    thread_->join();
}



void Thread::setDefaultName()
{
    int num= ++numCreated_;
    if(name_.empty())
    {
        char buf[32]={0};
        snprintf(buf,sizeof buf,"Tread%d",num);
        name_=buf;
    }
}
