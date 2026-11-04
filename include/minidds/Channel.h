#ifndef MINIDDS_CHANNEL_H
#define MINIDDS_CHANNEL_H

#include<functional>
#include<sys/epoll.h>

namespace minidds{
class EventLoop;

using EventCallback=std::function<void()>;

class Channel{
public:
    Channel(EventLoop* loop,int fd);
    ~Channel();

    int fd() const {return fd_;}
    int events() const {return events_;}
    void set_revents(int revents) {revents_=revents;}
    int revents() const {return revents_;}

    void setReadCallback(const EventCallback& cb) {readCallback_=cb;}
    void setWriteCallback(const EventCallback& cb) {writeCallback_=cb;}
    void setCloseCallback(const EventCallback& cb) {closeCallback_=cb;}

    void enableReading() {events_ |= EPOLLIN;update();}
    void enableWriting() {events_ |= EPOLLOUT;update();}
    void disableWriting() {events_ &= ~EPOLLOUT;update();}
    void disableAll() {events_ = 0;update();}

    bool isWriting() const {return events_ & EPOLLOUT;}

    void handleEvent();

    void remove();


private:
    void update();

    EventLoop* loop_;
    const int fd_;
    int events_=0;
    int revents_=0;
    bool added_=false;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
};
}



#endif