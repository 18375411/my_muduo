#include<mymuduo/TcpServer.hpp>
#include<mymuduo/logger.hpp>
#include<string>
#include<functional>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,const InetAdress &addr,const std::string name)
    :server_(loop,addr,name),loop_(loop)
    {
        //注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection,this,std::placeholders::_1)
        );

        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
        );

        //设置合适的loop线程数量
        server_.setThreadNumber(3);
    }

    void start()
    {
        server_.start();
    }
private:
    EventLoop *loop_;
    TcpServer server_;

    void onConnection(const TcpConnectionPtr &conn)
    {
        if(conn->connected())
        {
            LOG_INFO("Connected UP: %s",conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connected Down: %s",conn->peerAddress().toIpPort().c_str());
            
        }

    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf,Timestamp time)
    {
        std::string msg=buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown();
    }
};
int main()
{
    EventLoop loop;
    InetAdress addr(8000);
    EchoServer server(&loop,addr,"EchoServer-01");
    server.start();
    loop.loop();
    
    return 0;
}