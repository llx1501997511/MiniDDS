#include "minidds/Connection.h"
#include "minidds/EventLoop.h"
#include "minidds/Codec.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
namespace minidds {

Connection::Connection(EventLoop* loop,int fd)
    :loop_(loop),
    fd_(fd),
    Channel_(new Channel(loop,fd)){}

Connection::~Connection(){
    if(fd_>=0)::close(fd_);
}

void Connection::established(){
    Channel_->setReadCallback([this](){handleRead();});
    Channel_->setWriteCallback([this](){handleWrite();});
    Channel_->setCloseCallback([this](){handleClose();});

    Channel_->enableReading();
}

void Connection::destroy(){
    if(fd_<0)   return;
    Channel_->disableAll();
    Channel_->remove();
    ::close(fd_);
    fd_=-1;
    auto cb=std::move(closeCallback_);
    if(cb) cb();
}

void Connection::send(const std::string& data){
    outBuffer_+=data;
    if(!Channel_->isWriting()){
        Channel_->enableWriting();
    }
}

void Connection::subscribe(const std::string& topic){
    topics_.insert(topic);
}

void Connection::unsubscribe(const std::string& topic){
    topics_.erase(topic);
}

void Connection::handleRead(){
    char buf[65536];
    bool peerClosed=false;
    while(true){
        ssize_t n=::recv(fd_,buf,sizeof(buf),0);
        if(n>0){
            inBuffer_.append(buf,n);
        }
        else if(n==0){
            peerClosed=true;
            break;
        }else{
            if(errno==EAGAIN||errno==EWOULDBLOCK){
                break;
            }else if(errno==EINTR)
                continue;
            else{
                handleClose();
                return;
            }
            
        }
    }
    auto frames=Codec::decode(inBuffer_);
    for(const auto& f:frames){
        if(messageCallback_) messageCallback_(f);
    }
    //对端关闭也要让上面先发完缓冲区内容，再进行关闭。否则缓冲区内容都发不出去。sub都收不到后续内容
    
    if(peerClosed){
        handleClose();
    }
}

void Connection::handleWrite(){
    if(outBuffer_.empty()){
        Channel_->disableWriting();
        return;
    }
    size_t req = outBuffer_.size();//测试用：本次想要发多少，下面n是实际发多少
    ssize_t n=::send(fd_,outBuffer_.data(),outBuffer_.size(),MSG_NOSIGNAL);
    if(n>0){
        printf("[write] 请求=%zu 实发=%zd 剩余=%zu\n",req,n,req-(size_t)n);
        outBuffer_.erase(0,n);
        if(outBuffer_.empty()){
            Channel_->disableWriting();
        }
    }
    else if(n<0){
        if(errno==EAGAIN||errno==EWOULDBLOCK){
            printf("[write] 暂不可写 errno=%d 剩余=%zu\n",errno,req);
            return;
        }else{
            handleClose();
        }
    }
}

// TODO 空闲连接没做超时，现在只能等对端关闭或者出错
void Connection::handleClose(){
    destroy();
}


}