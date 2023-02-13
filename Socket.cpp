#include"Socket.hpp"

#include"logger.hpp"
#include"InetAdress.hpp"

#include <netinet/tcp.h>
#include<unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include<strings.h>
Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAdress(const InetAdress &localAdress)
{
    if(0!=bind(sockfd_,(sockaddr*)(localAdress).getSockAddr(),sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd:%d failed\n",sockfd_);
    }
}

void Socket::listen()
{
    if(0!=::listen(sockfd_,1024))
    {
        LOG_FATAL("listen sockfd:%d failed\n",sockfd_);
    }
}

int Socket::accept(InetAdress *peerAdress)
{
    sockaddr_in addr;
    socklen_t len= sizeof addr;
    bzero(&addr,sizeof(addr));
    int connfd=::accept4(sockfd_,(sockaddr*)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd>=0)
    {
        peerAdress->setSockAddr(addr);
    }

    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_,SHUT_WR)<0)
    {
        LOG_ERROR("socket::shutdownWrite failed\n");
    }
}


void Socket::setTcpNoDelay(bool on)
{
    int optval=on?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof optval);//j禁用Nagle算法（把小包组成成大包，提高带宽利用率）
}
void Socket::setReuseAddr(bool on)
{
    //如果端口忙，但TCP状态位于 TIME_WAIT ，可以重用端口。如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息， 指明"地址已经使用中"。
    int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);
}
void Socket::setReusePort(bool on)
{
    //SO_REUSEPORT用于多核环境下，允许多个线程或者进程绑定和监听同一个ip+port，无论UDP、TCP
     int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof optval);
}
void Socket::setKeepAlive(bool on)
{
    /**
     * 在长连接的服务器程序中，心跳包的设计很重要，通过心跳包可以感知客户端是否存活，以及是否能正常工作。
     * 
     * 关于SO_KEEPALIVE的含义是这样的：
     * 如果通信两端超过2个小时没有交换数据，那么开启keep-alive的一端会自动发一个keep-alive包给对端。
     * 如果对端正常的回应ACK包，那么一切都没问题，再等个2小时后发包(如果这两个小时仍然没有数据交换)。
     * 如果对端回应RST包，表明对端程序崩溃或重启，这边socket产生ECONNRESET错误，并且关闭。
     * 如果对端一直没回应，这边会每75秒再发包给对端，总共发8次共11分钟15秒。最后socket产生 ETIMEDOUT 错误，并且关闭。或者收到ICMP错误，表明主机不可到达，则会产生 EHOSTUNREACH 错误。
    */
     int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof optval);
}