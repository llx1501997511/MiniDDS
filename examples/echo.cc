#include "minidds/EventLoop.h"
#include "minidds/Channel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

using namespace minidds;

static void setNonBlocking(int fd){
    int flags=::fcntl(fd,F_GETFL,0);
    ::fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}

int main(){
    int listenfd=::socket(AF_INET,SOCK_STREAM,0);
    setNonBlocking(listenfd);
    int opt=1;
    ::setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(8000);
    ::bind(listenfd,(struct sockaddr *)&addr,sizeof(addr));
    ::listen(listenfd,128);

    EventLoop loop;
    Channel listenChannel(&loop,listenfd);
    listenChannel.setReadCallback([&](){
        struct sockaddr_in peer;
        socklen_t len=sizeof(peer);
        int connfd=::accept(listenfd,(struct sockaddr *)&peer,&len);
        if(connfd<0)    return;
        setNonBlocking(connfd);
        printf("[echo] accept fd=%d\n", connfd);

        Channel* conn=new Channel(&loop,connfd);
        conn->setReadCallback([conn](){
            char buf[1024];
            ssize_t n=::recv(conn->fd(),buf,sizeof(buf),0);
            if(n>0)
                ::send(conn->fd(),buf,n,0);
            else if(n==0){
                printf("[echo] close fd=%d\n", conn->fd());
                conn->remove();                    // 从 epoll 摘树
                ::close(conn->fd()); 
            }
        });
        conn->enableReading();

    });
    listenChannel.enableReading();
     printf("[echo] listening on 8000\n");
    loop.loop();   // 阻塞循环，
    return 0;
}