#include "minidds/Channel.h"
#include "minidds/EventLoop.h"

#include <cassert>
#include <cstdio>
#include <sys/epoll.h>

namespace minidds{
    Channel::Channel(EventLoop *loop,int fd)
        :loop_(loop),fd_(fd){}
    Channel::~Channel(){
        if(added_)
            remove();
    }

    void Channel::update(){
        int op=this->added_==true?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
        loop_->updateChannel(this,op);
        added_=true;
    }

    void Channel::remove(){
        if(!added_) return;
        loop_->removeChannel(this);
        added_=false;
    }

    void Channel::handleEvent(){
        if((revents_&(EPOLLHUP|EPOLLERR))&&!(revents_&EPOLLIN)){
            if(closeCallback_) closeCallback_();
            return;
        }
        if(revents_&EPOLLIN)
            if(readCallback_) readCallback_();
        if(revents_&EPOLLOUT)
            if(writeCallback_) writeCallback_();
    }



}