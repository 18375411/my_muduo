#include<muduo/net/TcpServer.h>//用于编写服务器程序
#include<muduo/net/EventLoop.h>
#include<iostream>
#include<functional>
#include<string>
using namespace std;
using namespace placeholders;
using namespace muduo;
using namespace  muduo::net;

//使用网络库开发的好处： 能够把网络I/O代码和业务代码区分开
//                                      用户的连接断开 用户的可读写事件
/**
 * 基于muduo网络库开发服务器程序
 * 1、组合TcpServer对象
 * 2、创建EventLoop事件循环对象的指针
 * 3、明确TcpServer构造函数需要的参数，输出ChatServer的构造函数
 * 4、在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写的回调函数
 * 5、设置线程数量，muduo库会自己分配IO线程和worker线程
*/
class ChatServer{
    public:
        ChatServer(EventLoop *loop, 
            const InetAddress& listenAddr,
            const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop){
                //给服务器创建用户连接的创建和断开回调
                _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));
                //给服务器注册用户读写事件回调
                _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));
                //设置服务器端的线程数量 1个IO线程，3个读写线程
                _server.setThreadNum(4);
            }
        //开启事件循环
        void start(){
            _server.start();
        }
    private:
    //专门处理用户连接创建和断开
    void onConnection(const TcpConnectionPtr& con){
        if(con->connected()){
            cout<<con->peerAddress().toIpPort()<<"->"<<con->localAddress().toIpPort()<<endl;
            cout<<"status:online"<<endl;

        }
        else
           {
            cout<<"status:offline"<<endl;
             con->shutdown();
        //    _loop->quit();
           }
    }
    //专门处理用户读写程序
    void onMessage(const TcpConnectionPtr& con,
                            Buffer*buffer,
                            Timestamp time){
                    string buf=buffer->retrieveAllAsString();
                    cout<<"recv data:"<<buf<<" time:"<<time.toString()<<endl;
                    con->send(buf);
                            }
    TcpServer _server;
    EventLoop *_loop; //epoll
};

int main(){
    cout<<"start:<<<<<<<<<<<<<"<<endl;
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");
    
    server.start();//listenfd ==> epollfd
    loop.loop();
    return 0;
}