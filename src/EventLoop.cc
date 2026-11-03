#include <minidds/EventLoop.h>
#include <minidds/Channel.h>
#include <minidds/Epoller.h>

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

namespace minidds{

namespace{
    long currentTid(){
        return ::syscall(SYS_getgid);
    }
}

EventLoop::EventLoop()
    :epoller_(new Epoller()),tid_(currentTid()){}
EventLoop::~EventLoop()=default;

bool EventLoop::isInLoopThread() const {
    return currentTid()==tid_;
}

void EventLoop::updateChannel(Channel *ch,int op){
    epoller_->updateChannel(ch,op);
}

void EventLoop::removeChannel(Channel *ch){
    epoller_->removeChannel(ch);
}

void EventLoop::loop(){
    looping_=true;
    quit_=false;

    while (!quit_)
    {
        activeChannels_.clear();
        activeChannels_=epoller_->poll(1000);
        for(Channel* ch:activeChannels_)
            ch->handleEvent();
    }
    looping_=false;
    
}

void EventLoop::quit()
{
    quit_=true;
}

}