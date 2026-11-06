//topic 名 → 订阅连接列表 的注册表
// 职责：维护「哪个 topic 被哪些连接订阅」的映射。
// 用一把大锁保护就可以 ，访问频率低、临界区短。
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