#pragma once

#include"noncopyable.hpp"
#include"InetAdress.hpp"
#include"Buffer.hpp"
#include"Callbacks.hpp"
#include"Timestamp.hpp"

#include<memory>
#include<functional>
#include<string>
#include<atomic>

class Channel;
class Socket;
class EventLoop;

/**
 * TcpSercer ----> Acceptor----->有一个新用户连接，调用accpet拿到connfd ----> TcpConnection设置回调 ----> Channel  ------>TcpConnection
*/

class TcpConnection: noncopybale,public std::enable_shared_from_this<TcpConnection>
{
public:
     enum StateE
    {
        kDisconnected,
        kConnectiong,
        kConnected,
        kDisconnecting
    };
    
    TcpConnection(EventLoop *loop
                ,const std::string nameArg
                ,int sockfd
                ,const InetAdress& localAddr
                ,const InetAdress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const {return loop_;}
    const std::string getName() const {return name_;}
    const InetAdress& localAddress() const{return localAddr_;}
    const InetAdress& peerAddress() const {return peerAddr_;}

    bool connected() const {return state_==kConnected;}
    void setState(StateE state){state_=state;}

    void send(const std::string &buf);//发送数据
    void shutdown();//关闭连接

    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
    void setMessageCallback(const MessageCallback& cb){messageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}
    void setCloseCallback(const CloseCallback& cb){closeCallback_=cb;}
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb){highWaterCallback_=cb;}

    //连接建立 
    void connectEstablished();
    //连接销毁
    void connetDestroyed();
private:
   

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *data,int len);
    void shutdownInLoop();


    EventLoop *loop_;//某个subLoop，因为TcpConnection都是在subLoop中管理
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    //这里和Acceptor类似，Acceptor=>mainLoop TcpConnection=>subLoop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAdress localAddr_;
    const InetAdress peerAddr_;

    ConnectionCallback connectionCallback_;//有新连接时的回调
    MessageCallback messageCallback_;//有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;//消息发送完成以后的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterCallback_;//高水位回调

    size_t highWaterMark_;

    Buffer inputBuffer_;//接收数据的缓冲区
    Buffer outputBuffer_;//发送数据的缓冲区

};