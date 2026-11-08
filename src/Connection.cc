// Connection.cc
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

Connection::~Connection()=default;

void Connection::established(){
    Channel_->setReadCallback([this](){handleRead();});
    Channel_->setWriteCallback([this](){handleWrite();});
    Channel_->setCloseCallback([this](){handleClose();});

    Channel_->enableReading();
}

void Connection::destroy(){
    Channel_->disableAll();
    Channel_->remove();
    ::close(fd_);
    if(closeCallback_) closeCallback_();
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
    //对端关闭也要让上面先发完缓冲区内容，再进行关闭，否则pub关闭，server这里直接关闭，
    //缓冲区内容都发不出去。sub都收到后续内容
    if(peerClosed){
        handleClose();
    }
}

void Connection::handleWrite(){
    if(outBuffer_.empty()){
        Channel_->disableWriting();
        return;
    }
    ssize_t n=::send(fd_,outBuffer_.data(),outBuffer_.size(),MSG_NOSIGNAL);
    if(n>0){
        outBuffer_.erase(0,n);
        if(outBuffer_.empty()){
            Channel_->disableWriting();
        }
    }
    else if(n<0){
        if(errno==EAGAIN||errno==EWOULDBLOCK){
            return;
        }else{
            handleClose();
        }
    }
}

void Connection::handleClose(){
    if(closeCallback_) closeCallback_();
}


}