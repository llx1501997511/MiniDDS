# MiniDDS

轻量级发布订阅通信中间件：进程之间不直接通信，只按 topic 名字收发。
基于 TCP + epoll + Reactor，消息体用 Protobuf 序列化，4 字节长度前缀分帧。

## 特性

- 事件驱动网络层（epoll + Reactor，单 IO 线程）
- 长度前缀分帧，解决 TCP 粘包/半包
- Topic 发布订阅（SUB / UNSUB / PUB）
- Protobuf 序列化
- 内置线程池组件（ThreadPool，固定线程数）

## 构建

```bash
mkdir -p build && cd build
cmake .. && make -j4
```

## 运行

```bash
# 终端 1：服务器
./minidds_server 8000
# 终端 2：订阅 imu
./subscriber 8000 imu
# 终端 3：发布 5 条 imu 消息
./publisher 8000 imu 5
```

## 目录结构

```
minidds/
├─ proto/      消息定义（protobuf）
├─ include/    头文件
├─ src/        实现
├─ examples/   发布者 / 订阅者示例
└─ tests/      单元测试
```
