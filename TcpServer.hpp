#pragma once

/**
 * 用户使用muduo编写服务器
*/
#include"noncopyable.hpp"
#include"EventLoop.hpp"
#include"InetAdress.hpp"
#include"Acceptor.hpp"
#include"EventLoopThreadPool.hpp"
#include"TcpConnection.hpp"
#include"Callbacks.hpp"
#include"Buffer.hpp"

#include<functional>
#include<string>
#include<memory>
#include<atomic>
#include<unordered_map>
class TcpServer:noncopybale
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };
    TcpServer(EventLoop *loop
            ,const InetAdress &listenAddr
            ,const std::string nameArg
            ,Option option=kNoReusePort );
    ~TcpServer();


    void setThreadNumber(int numThreads);

    //开启服务器监听
    void start();

    void setThreadInitCallback(const ThreadInitCallback &cb){threadInitCallback_=cb;}
    void setConnectionCallback(const ConnectionCallback &cb){connectionCallback_=cb;}
    void setMessageCallback(const MessageCallback &cb){messageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb){writeCompleteCallback_=cb;}

private:
    void newConnection(int sockfd,const InetAdress &peerAddr);
    void removeConnetion(const TcpConnectionPtr &conn);
    void removeConnetionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;

    EventLoop *loop_;//用户传递的loop（mainLoop，运行Acceptor监听新用户连接）

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;//mainLoop acceptor 监听新用户连接

    std::unique_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;//有新连接时的回调
    MessageCallback messageCallback_;//有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;//消息发送完成以后的回调
    
    ThreadInitCallback threadInitCallback_;//新建线程时在新线程中调用的initCallback

    std::atomic_int  started_;

    int nextConnId_;
    ConnectionMap connections_;//保存所有连接

};

