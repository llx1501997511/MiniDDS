// topic -> 订阅了这个 topic 的连接集合

#ifndef MINIDDS_TOPICREGISTRY_H
#define MINIDDS_TOPICREGISTRY_H

#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <string>

namespace minidds{
class Connection;

class TopicRegistry{
public:
    //订阅
    void subscribe(const std::string& topic,Connection* conn);
    //取消订阅
    void unsubscribe(const std::string& topic,Connection* conn);
    //移除一个连接，消除它所有的订阅
    void removeConnection(Connection* conn);
    //得到一个订阅的所有连接拷贝副本
    std::vector<Connection*> getSubscribers(const std::string& topic);
private:
    std::mutex mutex_;
    std::map<std::string,std::set<Connection*>> topicToConns_;
};

}

#endif