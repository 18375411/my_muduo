#include"TcpConnection.hpp"
#include"logger.hpp"
#include"Socket.hpp"
#include"Channel.hpp"
#include"EventLoop.hpp"

#include<errno.h>
#include <netinet/tcp.h>
#include<unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>

static EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if(loop==nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnetcion Loop is null\n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop
                ,const std::string nameArg
                ,int sockfd
                ,const InetAdress& localAddr
                ,const InetAdress& peerAddr)
                :loop_(CheckLoopNotNull(loop))
                ,name_(nameArg)
                ,state_(kConnectiong)
                ,reading_(true)
                ,socket_(new Socket(sockfd))
                ,channel_(new Channel(loop,sockfd))
                ,localAddr_(localAddr)
                ,peerAddr_(peerAddr)
                ,highWaterMark_(64*1024*1024)//64M
{
    //给channel设置相应的回调函数
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));

    LOG_INFO("TcpConnection::ctor[%s] at fd:%d\n",name_.c_str(),sockfd);
    socket_->setKeepAlive(true);//启动Tcp的保活机制
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd:%d state=%d\n",name_.c_str(),channel_->fd(),(int)state_);
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrono=0;
    size_t n= inputBuffer_.readfd(channel_->fd(),&savedErrono);
    if(n>0)
    {
        //已建立连接的用户有可读事件发生了，调用用户传递的回调函数
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        errno=savedErrono;
        LOG_ERROR("TcpConnection::handleRead error:%d\n",errno);
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if(channel_->isWring())
    {
        int savedErrno=0;
        ssize_t n=outputBuffer_.writefd(channel_->fd(),&savedErrno);
        if(n>0)
        {
            outputBuffer_.retrive(n);
            if(outputBuffer_.readableBytes()==0)//数据已经写完
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this()));
                }
            }
            if(state_==kConnectiong)//发送过程中调用了shutdown
            {
                shutdownInLoop();
            }
        }
        else
        {
            LOG_ERROR("Tcpconnection::handleWrite errno;\n");
        }
    }
    else
    {
        LOG_ERROR("Tcpconnection fd=%d is down, no more writing \n",channel_->fd());
    }
}

//poller ==> channel::closeCallback => TcpConnection::handleClose()
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n",channel_->fd(),(int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    if(connectionCallback_)
    {
        connectionCallback_(connPtr);//执行连接关闭的回调
    }
    if(closeCallback_)
    {
        closeCallback_(connPtr);//执行的是TcpServer::removeConnetion,removeConnetion中执行了TcpConnection::connetDestroyed
    }
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen=sizeof optval;
    int err=0;
    // 功能：获取一个套接字的选项
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0)
    {
        err=errno;
    }
    else
    {
        err=optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s -SO_ERROR:%d \n",name_.c_str(),errno);
}

/**
 * 应用写的快，但内核发送的慢，因此需要把待发送的数据写入缓冲区，并且设置了高水位
*/
void TcpConnection::sendInLoop(const void *data,int len)
{
    ssize_t nwrote=0;
    ssize_t remaining=0;
    bool faultError=false;

    //之前调用过该Connection的shutdown，不能再发送了
    if(state_==kDisconnected)
    {
        LOG_ERROR("disconnected,give up writng\n");
        return;
    }

    //channel_第一次开始写数据，且缓冲区无待发送数据
    if(!channel_->isWring() && outputBuffer_.readableBytes()==0)
    {
        nwrote=::write(channel_->fd(),data,len);
        if(nwrote>0)
        {
            remaining=len-nwrote;
            //既然在这里数据全部发送完了，就不用再给channel设置epollout事件了，也就不需要channel_再执行handlewrite回调了
            if(remaining==0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this()));
            }
        }
        else//nwrote<0
        {
            nwrote=0;
            if(errno!=EWOULDBLOCK)//EWOULDBLOCK：用于非阻塞模式，不需要重新读或者写
            {
                LOG_ERROR("TcpConnection::sendInLoop error");
                //往一个写端关闭的管道或socket连接中连续写入数据时会引发SIGPIPE信号
                /**ECONNRESET:
                 * - 接收端recv或者read， 对端已经关闭连接，recv/read返回该错误
                 * - 对端重启连接，还未建立连接
                 * - 发送端已经断开连接，但是调用send会触发这个错误
                */
                if(errno==EPIPE||errno==ECONNRESET)
                {
                    faultError=true;
                }
            }
        }
    }


    //当前这一次write没有全部发送出去，所以剩余的数据需要保存到缓冲区中，并给channel注册给epollout事件
    //poller发现tcp的发送缓冲区有数据，会通知相应的channel，调用channel的writeCallback
    //也就是 TcpConnection::handleWrite，把发送缓冲区数据全部发送完成
    if(!faultError && remaining>0)
    {
        size_t oldLen=outputBuffer_.readableBytes();//目前发送缓冲区待发送的数据
        //oldLen > highWaterMark_ 则说明上次已经执行过了highWaterCallback_
        if(remaining+oldLen>=highWaterMark_ && oldLen<highWaterMark_ && highWaterCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterCallback_,shared_from_this(),oldLen+remaining));
        }
        outputBuffer_.append((char*)data+nwrote,remaining);
        if(!channel_->isWring())
        {
            channel_->enbleWriting();//一定要注册channel_的写事件，否则无法通过poller调用写回调
        }
    }
}


void TcpConnection::shutdown()
{
    if(state_==kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
}


void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWring())//当前OutputBuffer中的数据已经全部发送完成
    {
        socket_->shutdownWrite();//给channel触发了EPOLLHUP事件,channel会执行closeCallback，即TcpConnection::handleClose
    }
}


void TcpConnection::send(const std::string &buf)
{
    if(state_==kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(),buf.size());

        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,buf.c_str(),buf.size()));
        }
    }
}


//连接建立 
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enbleReading();

    //新连接建立，执行回调
    connectionCallback_(shared_from_this());
    
}
//连接销毁
void TcpConnection::connetDestroyed()
{
    if(state_==kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
        
    }
    channel_->remove();//从poller中删除channel
}