#include "minidds/EventLoop.h"
#include "minidds/Connection.h"
#include "minidds/Channel.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace minidds;

static void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
int main() {
    // 关掉 stdout 缓冲，让日志实时打印
    setvbuf(stdout, NULL, _IONBF, 0);
    EventLoop loop;
    // 建监听 socket
    int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));  // 端口复用，重启不报「地址被占」
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;   // 监听所有网卡
    addr.sin_port = htons(8000);
    ::bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    ::listen(listenfd, 16);
    setNonBlocking(listenfd);
  
    std::unordered_map<int, std::shared_ptr<Connection>> conns;
    // 监听 socket 也挂进 loop，读事件 = 有新连接进来
    Channel listenChan(&loop, listenfd);
    listenChan.setReadCallback([&]() {
        while (true) {
            int connfd = ::accept(listenfd, nullptr, nullptr);
            if (connfd < 0) break;          // EAGAIN：接完了，退出循环
            setNonBlocking(connfd);
            auto conn = std::make_shared<Connection>(&loop, connfd);
            // echo 核心：收到一条完整消息，原样回给对端
        
            conn->setMessageCallback([conn](const std::string& payload) {
                conn->send(payload);
            });
           
           
            conn->setCloseCallback([&, connfd]() {
                conns.erase(connfd);
            });
            conns[connfd] = conn;
            conn->established();            // 建连：设三个回调 + 开始读
            printf("[echo] accept fd=%d\n", connfd);
        }
    });
    listenChan.enableReading();
    printf("[echo] 监听 8000，测试：echo hello | nc 127.0.0.1 8000\n");
    loop.loop();   
    return 0;
}