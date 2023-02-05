#pragma once


#include<vector>
#include<string>
#include<algorithm>
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
//网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapaPrenpend=8;
    static const size_t kInitialSize=1024;

    explicit Buffer(size_t initialSize=kInitialSize)
        :buffer_(kCheapaPrenpend+initialSize)
        ,readIndex_(kCheapaPrenpend)
        ,writerIndex_(kCheapaPrenpend)
    {
    }

    size_t readableBytes() const
    {
        return writerIndex_-readIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size()-writerIndex_;
    }

    size_t prependableSize() const
    {
        return readIndex_;
    }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin()+readIndex_;
    }

    void retrive(size_t len)
    {
        if(len<readableBytes())
        {
            readIndex_+=len;//应用只读取了可读缓冲区数据的一部分，即len长度
        }
        else //len==readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readIndex_=kCheapaPrenpend;
        writerIndex_=kCheapaPrenpend;
    }

    //把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);//将数据存储在了result中
        retrive(len);//上面已经将缓冲区中的可读数据读取出来了，因此要做复位操作
        return result;
    }

    void ensureWritableBytes(size_t len)//确保有足够的写空间
    {
        if(writableBytes()<len)//需要扩容
        {
            makeSpace(len);
        }
    }

    //把（data,data+len)上的数据添加到write缓冲区上
    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_+=len;
    }

    char* beginWrite()
    {
        return begin()+writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin()+writerIndex_;
    }

    //从fd上读取数据
    ssize_t readfd(int fd,int *saveErrno);
private:
    char* begin()
    {
        return &*buffer_.begin();//1.*buffer_.begin()返回vector首元素 2.取地址 即返回数组的起始地址
    }
    const char *begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        if(writableBytes()+prependableSize() <len+kCheapaPrenpend)//前面空闲的加后面可写的位置都不够要写的
        {
            buffer_.resize(writerIndex_+len);
        }
        else//可以通过移动前面空闲区来容纳
        {
            size_t readable=readableBytes();
            std::copy(begin()+readIndex_,begin()+writerIndex_,begin()+kCheapaPrenpend);
            readIndex_=kCheapaPrenpend;
            writerIndex_=kCheapaPrenpend+readable;
        }
        
    }
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writerIndex_;
};