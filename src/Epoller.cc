#include <minidds/Epoller.h>
#include <minidds/Channel.h>

#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <cerrno>

namespace minidds{

namespace{
    const int KInitEventListSize=1024;
}

Epoller::Epoller(){
    epollfd_=epoll_create1(EPOLL_CLOEXEC);
    if(epollfd_<0){
        perror("epoll_create1");
        std::abort();
    }
    events_.resize(KInitEventListSize);
}

Epoller::~Epoller(){
    ::close(epollfd_);
}

void Epoller::updateChannel(Channel* ch,int op){
    struct epoll_event ev;
    memset(&ev,0,sizeof(ev));
    ev.events=ch->events();
    ev.data.ptr=ch;

    if(::epoll_ctl(epollfd_,op,ch->fd(),&ev)<0){
        perror("epoll_ctl");
    }
}  

void Epoller::removeChannel(Channel* ch){
    struct epoll_event ev;
    memset(&ev,0,sizeof(ev));
    if(::epoll_ctl(epollfd_,EPOLL_CTL_DEL,ch->fd(),&ev)<0){
        perror("epoll_ctl DEL");
    }
}

Epoller::ChannelList Epoller::poll(int timeoutMs){
    int n=::epoll_wait(epollfd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
    ChannelList ret;
    if(n<0){
        if(errno==EINTR) return ret;   // 被信号打断，正常返回
        perror("epoll_wait");
        return ret;
    }

    for(int i=0;i<n;i++){
        Channel* ch=static_cast<Channel *>(events_[i].data.ptr);
        ch->set_revents(events_[i].events);
        ret.push_back(ch);
    }

    if(n==static_cast<int>(events_.size())){
        events_.resize(events_.size()*2);
    }
    return ret;

}

} //minidds