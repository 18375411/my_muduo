#include"Buffer.hpp"
 
#include<errno.h>
#include<sys/uio.h>
#include<unistd.h>
/**
 * 从fd上读数据，poller工作在LT模式
 * Buffer缓冲区是有大小的，但是从fd上读数据的时候，不知道tcp数据的最终大小
*/
ssize_t Buffer::readfd(int fd,int *saveErrno)
{
    char extrabuf[65536]={0};//栈上的空间 64K
    
    struct iovec vec[2];
    //readv函数将数据从文件描述符读到分散的内存块中，即分散读；writev函数将多块分散的内存数据一并写入文件描述符中，即集中写
    const size_t writable=writableBytes();//Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base=begin()+writerIndex_;
    vec[0].iov_len=writable;

    vec[1].iov_base=extrabuf;
    vec[2].iov_len=sizeof extrabuf;

    const int iovcnt=(writable<sizeof extrabuf)?2:1;
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;
    }
    else if(n<=writable)//Buffer的可写缓冲区已经够存储读取的数据了
    {
        writerIndex_+=n;
    }
    else//extrabuf也写入了数据
    {
        writerIndex_=buffer_.size();
        append(extrabuf,n-writable);
    }
    return n;
}


ssize_t Buffer::writefd(int fd,int *saveErrno)
{
    ssize_t n=::write(fd,peek(),readableBytes());
    if(n<0)
    {
        *saveErrno=errno;
    }
    return n;
}
