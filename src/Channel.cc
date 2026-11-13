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

    // 回调里可能把 this 删掉，所以先把 revents 存到局部变量里
    void Channel::handleEvent(){
        const int re=revents_;
        if((re&(EPOLLHUP|EPOLLERR))&&!(re&EPOLLIN)){
            EventCallback cb=closeCallback_;
            if(cb) cb();
            return;
        }
        if(re&EPOLLIN){
            EventCallback cb=readCallback_;
            if(cb) cb();
        }
        if(re&EPOLLOUT){
            EventCallback cb=writeCallback_;
            if(cb) cb();
        }
    }



}
