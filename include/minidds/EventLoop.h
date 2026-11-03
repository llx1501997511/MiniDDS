#ifndef MINIDDS_EVENTLOOP_H
#define MINIDDS_EVENTLOOP_H

#include <functional>
#include <memory>
#include <vector>

namespace minidds{
class Channel;
class Epoller;

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel* ch,int op);
    void removeChannel(Channel* ch);

    bool isInLoopThread() const;
private:
    bool looping_=false;
    bool quit_=false;
    std::unique_ptr<Epoller> epoller_;
    std::vector<Channel*> activeChannels_;
 
    const long tid_;
};
    
} // namespace minidds{class Channel

#endif