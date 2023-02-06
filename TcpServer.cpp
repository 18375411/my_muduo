#include"TcpServer.hpp"
#include"logger.hpp"

#include<strings.h>

static EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if(loop==nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null\n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop
            ,const InetAdress &listenAddr
            ,const std::string nameArg
            ,Option option)
            :loop_(CheckLoopNotNull(loop))
            ,ipPort_(listenAddr.toIpPort())
            ,name_(nameArg)
            ,acceptor_(new Acceptor(loop,listenAddr,option==kReusePort))
            ,threadPool_(new EventLoopThreadPool(loop,name_))
            ,connectionCallback_()
            ,messageCallback_()
            ,nextConnId_(1)
{
    //当有新用户连接时，将执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));

}


TcpServer::~TcpServer()
{
    for(auto & item:connections_)
    {
        //这个局部的shared_ptr对象，出右括号，可以自动释放new出来的TcpConnection对象
        TcpConnectionPtr conn(item.second);
        /**当智能指针调用了reset函数的时候，就不会再指向这个对象了。
        * 如果还有其它智能指针指向这个对象，那么其他的智能指针的引用计数会减1。
        * */
        item.second.reset();

        //销毁连接
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connetDestroyed,conn));

    }
}

//有一个新的客户端的连接时，acceptor会执行该方法
void TcpServer::newConnection(int sockfd,const InetAdress &peerAddr)
{   
    //轮询算法选择一个subLoop
    EventLoop *ioLoop=threadPool_->getNextLoop();
    char buf[64]={0};
    snprintf(buf,sizeof buf,"-%s#%d",ipPort_.c_str(),nextConnId_);
    nextConnId_++;
    std::string connName=name_+buf;

    LOG_INFO("TcpServer::newConnection [%s] - new Connection [%s] from %s \n",
        name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    //通过sockfd获取其绑定的本机IP地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof local);
    socklen_t addrlen=sizeof local;
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen)<0)
    {
        LOG_ERROR("sockets::getsockname\n");
    }
    InetAdress localAddr(local);

    //根据连接成功的sockfd，创建Tcpconnection对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
    connections_[connName]=conn;
    
    //下面的回调都是用户设置给TcpServer的
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //设置了如何关闭该连接的回调
    conn->setCloseCallback(std::bind(&TcpServer::removeConnetion,this,std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));

}
void TcpServer::removeConnetion(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnetionInLoop,this,conn));
}
void TcpServer::removeConnetionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO(" TcpServer::removeConnetionInLoop [%s] -connection[%s]\n",
        name_.c_str(),conn->getName().c_str());
    
    connections_.erase(conn->getName());
    EventLoop* ioLoop=conn->getLoop();

    ioLoop->queueInLoop(std::bind(&TcpConnection::connetDestroyed,conn));
}


void TcpServer::setThreadNumber(int numThreads)
{
    threadPool_->setThreadNumber(numThreads);
}

//开启服务器监听
void TcpServer::start()
{
    if(started_++==0)//防止一个TcpServer对象被start多次
    {
        threadPool_->start(threadInitCallback_);//启动底层线程池
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));//当前就是在主线程中调用runInLoop，因此直接执行了Acceptor::listen

    }
}