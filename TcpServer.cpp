#include"TcpServer.hpp"
#include"logger.hpp"

EventLoop* CheckLoopNotNull(EventLoop *loop)
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


void TcpServer::newConnection(int sockfd,const InetAdress &peerAddr)
{

}
void TcpServer::removeConnetion(const TcpConnectionPtr &conn)
{

}
void TcpServer::removeConnetionInLoop(const TcpConnectionPtr &conn)
{

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