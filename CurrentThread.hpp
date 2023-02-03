#pragma once

namespace CurrentThead
{
    extern __thread int t_cathedTid;
    
    void catchTid();

    inline int tid()//内联函数只在当前文件起作用
    {   
        if(__builtin_expect(t_cathedTid==0,0))//gcc引入的，作用是"允许程序员将最有可能执行的分支告诉编译器"。这个指令的写法为：__builtin_expect(EXP, N)。意思是：EXP==N的概率很大。
        {
            catchTid();
        }
        return t_cathedTid;
    }
}