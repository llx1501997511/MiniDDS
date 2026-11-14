// 一条连接 = fd + 读写缓冲 + 它订阅的 topic，缓冲和 Channel 只在 IO 线程里碰

#ifndef MINIDDS_CONNECTION_H
#define MINIDDS_CONNECTION_H
#include <memory>
#include <set>
#include <string>
#include "minidds/Channel.h"

namespace minidds{

class EventLoop;
class Connection:public std::enable_shared_from_this<Connection>{
public:
    Connection(EventLoop* loop,int fd);
    ~Connection();

    int fd() const {return fd_;}
    EventLoop* loop() const{ return loop_;}

    void established();
    void destroy();

    void send(const std::string& data);

    void subscribe(const std::string& topic);
    void unsubscribe(const std::string& topic);
    const std::set<std::string>& topics() const {return topics_;}

    using MessageCallback = std::function<void(const std::string&)>;
    void setMessageCallback(const MessageCallback& cb) {messageCallback_=cb;}
    void setCloseCallback(const EventCallback& cb) {closeCallback_=cb;}

private:
    void handleRead();
    void handleWrite();
    void handleClose();

    EventLoop* loop_;
    int fd_;
    std::unique_ptr<Channel> channel_;

    std::string inBuffer_;
    std::string outBuffer_;
    
    std::set<std::string> topics_;

    MessageCallback messageCallback_;
    EventCallback closeCallback_;

};

}

#endif