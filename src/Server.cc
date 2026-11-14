#include "minidds/Server.h"
#include "minidds/Channel.h"
#include "minidds/Connection.h"
#include "minidds/Codec.h"
#include "minidds/TopicRegistry.h"
#include "minidds/ThreadPool.h"
#include "message.pb.h"  // protoc 生成
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
namespace minidds {

namespace{

void setNonBlocking(int fd){
    int flags=::fcntl(fd,F_GETFL,0);
    ::fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}

}

Server::Server(EventLoop* loop,int port):loop_(loop),port_(port)
    ,registry_(new TopicRegistry()),pool_(new ThreadPool(4)){}

Server::~Server(){
    if(listenFd_>=0)::close(listenFd_);
}

bool Server::start(){
    listenFd_=::socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_<0){
        perror("socket");
        return false;
    }
    setNonBlocking(listenFd_);

    int opt=1;
    ::setsockopt(listenFd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port_);

    if(::bind(listenFd_,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("bind");
        return false;
    }

    if(::listen(listenFd_,128)<0){
        perror("listen");
        return false;
    }

    listenChannel_=std::make_unique<Channel>(loop_,listenFd_);
    listenChannel_->setReadCallback([this](){handleAccept();});
    listenChannel_->enableReading();

    printf("[Server] listening on port %d\n",port_);
    return true;

}

void Server::handleAccept(){
    struct sockaddr_in perr;
    socklen_t len=sizeof(perr);
    while(true){
        int connfd=::accept(listenFd_,(struct sockaddr*)&perr,&len);
        if(connfd<0){
            if(errno==EAGAIN||errno==EWOULDBLOCK){
                break;
            }
            perror("accept");
            break;
        }
        setNonBlocking(connfd);

        auto conn=std::make_shared<Connection>(loop_,connfd);
        int fd=conn->fd();
        conns_[fd]=conn;

        conn->setMessageCallback([this,p=conn.get()](const std::string& payload)
        {onMessage(p,payload);});

        conn->setCloseCallback([this,fd](){
            if(conns_.count(fd)){
                registry_->removeConnection(conns_[fd].get());
                conns_.erase(fd);
            }
        });

        conn->established();
        printf("[Server] accept fd=%d\n",connfd);
    }
}

void Server::onMessage(Connection* conn,const std::string& payload){
    Envelope env;
    if(!env.ParseFromString(payload)){
        printf("[Server] fd=%d 解包失败，丢弃\n",conn->fd());
        return;
    }

    if(env.has_control()){
        const Control& ctrl =env.control();
        switch (ctrl.type()){
        case Control::SUB:
            conn->subscribe(ctrl.topic());
            registry_->subscribe(ctrl.topic(),conn);
            printf("[Server] fd=%d SUB %s\n",conn->fd(),ctrl.topic().c_str());
            break;
        case Control::UNSUB:
            conn->unsubscribe(ctrl.topic());
            registry_->unsubscribe(ctrl.topic(),conn);
            printf("[Server] fd=%d UNSUB %s\n",conn->fd(),ctrl.topic().c_str());
            break;
        default:
            break;         
        }
        return;
    }
    if(!env.has_message())
        return;
    
    
    const Message& msg=env.message();
    auto subs=registry_->getSubscribers(msg.topic());
    std::string frame=Codec::encode(payload);
    for(Connection* sub:subs){
        if(sub==conn)   continue;

        //业务处理，投递到线程池（打印tid，证明解耦）
        pool_->enqueue([frame,topic=msg.topic(),seq=msg.seq()](){
            printf("[业务线程 %ld] 开始 topic=%s seq=%lu\n",
                (long)gettid(),topic.c_str(),(unsigned long)seq);
            
            printf("[业务线程 %ld] 完成 topic=%s seq=%lu\n",
                (long)gettid(),topic.c_str(),(unsigned long)seq);
        });

        sub->send(frame);
    }

    printf("[Server] PUB topic=%s seq=%lu -> %zu 个订阅者\n",
        msg.topic().c_str(),(unsigned long)msg.seq(),subs.size());
}

}
