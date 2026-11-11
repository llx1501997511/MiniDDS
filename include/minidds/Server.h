// 监听 + 接入 + 分发


#ifndef MINIDDS_SERVER_H
#define MINIDDS_SERVER_H
#include <memory>
#include <string>
#include <unordered_map>
#include "minidds/EventLoop.h"
namespace minidds {

class Channel;
class Connection;
class TopicRegistry;
class ThreadPool;

class Server{
public:
    Server(EventLoop* loop,int port);
    ~Server();

    void start();

private:
    void handleAccept();

    void onMessage(Connection* conn,const std::string& payload);

    EventLoop* loop_;
    int listenFd_;
    int port_;

    std::unique_ptr<TopicRegistry> registry_;
    std::unique_ptr<ThreadPool> pool_;
    std::unique_ptr<Channel> listenChannel_;

    std::unordered_map<int,std::shared_ptr<Connection>> conns_;
};


}
#endif