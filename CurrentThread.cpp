#include"CurrentThread.hpp"

#include<unistd.h>
#include<sys/syscall.h>

namespace CurrentThread
{
    __thread int t_cathedTid=0;//__thread表明是该线程独立实体

    void catchTid()
    {
        if(t_cathedTid==0)
        {
            //通过linux系统调用获取该线程的tid值
            t_cathedTid=static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}