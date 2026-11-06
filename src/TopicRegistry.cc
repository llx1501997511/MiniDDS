#include "minidds/TopicRegistry.h"
#include "minidds/Connection.h"

namespace minidds{
    void TopicRegistry::subscribe(const std::string& topic,Connection* conn){
        std::lock_guard<std::mutex> lock(mutex_);
        topicToConns_[topic].insert(conn);
    }

    void TopicRegistry::unsubscribe(const std::string& topic,Connection* conn){
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = topicToConns_.find(topic);
        if(it!=topicToConns_.end()){
            it->second.erase(conn);
            if(it->second.empty()){
                topicToConns_.erase(it);
            }
        }
    }

    void TopicRegistry::removeConnection(Connection* conn){
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it=topicToConns_.begin();it!=topicToConns_.end();){
            it->second.erase(conn);
            if(it->second.empty()){
                it = topicToConns_.erase(it);
            }else
                it++;
        }
    }

    std::vector<Connection*> TopicRegistry::getSubscribers(const std::string& topic){
        std::vector<Connection*> result;
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=topicToConns_.find(topic);
        if(it!=topicToConns_.end()){
            result.assign(it->second.begin(),it->second.end());
        }
        return result;
    }

}