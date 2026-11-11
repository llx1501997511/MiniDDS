# MiniDDS

自己写的一个发布订阅中间件：客户端之间不直接连，只把消息发到 topic 上，由服务器按订阅关系转发。
网络层 TCP + epoll + Reactor，消息体 Protobuf，帧头 4 字节长度前缀。

做这个的目的就是把中间件在干什么自己走一遍：epoll 怎么管连接、粘包怎么切、订阅表怎么维护、
慢的收端怎么不拖垮别人。网络层（epoll、Reactor、分帧、线程池）都是自己写的，没有套 muduo
这类现成框架；序列化用 protobuf。

## 架构

单 IO 线程，accept 之后把 fd 包成 Channel 挂进 epoll：

    accept -> Channel -> epoll_wait -> 可读 -> 边收边切帧 -> 分两类
                                                          ├ 控制帧 SUB/UNSUB -> 改订阅表
                                                          └ 数据帧 PUB -> 查订阅表 -> 逐个 send

几个约定：

- 一条连接 = fd + 读缓冲 + 写缓冲 + 它订阅的 topic 集合
- 缓冲区和 Channel 只在 IO 线程里碰，别的线程不碰
- 业务回调丢进线程池，不占 IO 线程

## 特性

- epoll + Reactor，IO 线程只有一个，不需要给缓冲加锁
- 4 字节长度前缀分帧，粘包半包都在 Codec 里切
- SUB / UNSUB / PUB，一个连接可以同时订阅多个 topic
- protobuf oneof 区分控制消息和数据消息，收到乱字节不会误判
- 写缓冲没发完就挂上 EPOLLOUT，发空了再摘掉
- 固定线程数的线程池跑业务回调

## 构建

```bash
mkdir -p build && cd build
cmake .. && make -j4
```

## 运行

```bash
# 终端 1
./minidds_server 8000
# 终端 2
./subscriber 8000 imu
# 终端 3
./publisher 8000 imu 5
```

## 测试

单个模块的测试手工编译，在仓库根目录：

```bash
g++ -std=c++17 -Iinclude tests/test_codec.cc src/Codec.cc -o build/test_codec && ./build/test_codec
```

- `tests/test_codec.cc` 单帧、半包、粘包、超长帧拒绝，全用 assert
- `tests/proto_test.cc` 序列化往返、oneof 判定、乱字节不误判
- `tests/topic_test.cc` 重复订阅去重、removeConnection 一次清掉所有 topic
- `tests/stress_multi.sh` 并发压测

压测 40 个订阅者 + 10 个发布者同时发：

    每个订阅者应该收 10 x 10 = 100 条，实测 40 个全是 100，一共 4000 条没漏
    再杀掉一半订阅者、补发一条探针，服务器记的订阅数正确掉到 20，没崩

10MB 大包那组是验短写的：一次 send 发不完，strace 里能看到 4 次 sendto
（2614080 / 3732531 / 3470599 / 668569 字节），剩下的留在写缓冲里等 EPOLLOUT。
所以 send 返回了不等于对端收到了，写缓冲和 EPOLLOUT 不是可选项。

## 已知限制

- 不存历史消息，订阅者必须先连上再发，先发后订的那条就丢了
- 业务回调现在只打印 tid，没有真实业务
- 压测脚本只数条数，验不了消息有没有串到别的 topic
- 没有空闲连接超时，也没有鉴权
- IO 只有一个线程，连接数上去了要拆成多线程

## 目录

```
minidds/
├─ proto/      消息定义
├─ include/    头文件
├─ src/        实现 + main
├─ examples/   publisher / subscriber 示例
└─ tests/      单元测试 + 压测脚本
```
