#include"Acceptor.hpp"
#include"logger.hpp"
#include"InetAdress.hpp"

#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include<errno.h>
#include<unistd.h>



static int creatNonBlocking()
{
    int sockfd=::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
    if(sockfd<0)
    {
        LOG_FATAL("%s:%s:%d listen socket create error:%d\n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop,const InetAdress &listenAddr,bool reusePort)
    :loop_(loop)
    ,acceptSocket_(creatNonBlocking())
    ,acceptChannel_(loop,acceptSocket_.fd())
    ,lisenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptSocket_.bindAdress(listenAddr);
    //TcpServer.listen() Acceptor.listen 有新用户连接 ==》执行回调==》 分配subloop
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}


void Acceptor::listen()
{
    lisenning_=true;
    acceptSocket_.listen();
    acceptChannel_.enbleReading();

}

//listenfd有事件发生了，就是有新用户连接了
void Acceptor::handleRead()
{
    InetAdress peerAddr;
    int connfd=acceptSocket_.accept(&peerAddr);
    if(connfd>=0)
    {
        if(newConnectionCallback_)
            newConnectionCallback_(connfd,peerAddr);//轮询找到subloop，唤醒，分发当前新客户端的channel
        else
            ::close(connfd);
    }
    else
    {
        LOG_ERROR("%s:%s:%d accpet error:%d\n",__FILE__,__FUNCTION__,__LINE__,errno);
        if(errno==ENFILE)
        {
            //增加sockfd连接上限 or 集群or分布式部署
            LOG_ERROR("%s:%s:%d sockfd accpet reached limit\n",__FILE__,__FUNCTION__,__LINE__);

        }
    }
}