#ifndef MINIDDS_EPOLLER_H
#define MINIDDS_EPOLLER_H

#include <vector>
#include <sys/epoll.h>

namespace minidds{
class Channel;
    
class Epoller{
public:
    using ChannelList=std::vector<Channel *>;

    Epoller();
    ~Epoller();

    void updateChannel(Channel* ch,int op);

    void removeChannel(Channel* ch);

    ChannelList poll(int timeoutMs);
private:
    int epollfd_;
    std::vector<struct epoll_event> events_;
};
} //namespace minidds

#endif