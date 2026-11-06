// Day 5 验证：TopicRegistry 订阅表

#include "minidds/TopicRegistry.h"

#include <cstdint>
#include <cstdio>

using minidds::Connection;
using minidds::TopicRegistry;

int main() {
    // 两个「假连接」，只要地址不同就能区分
    Connection* a = reinterpret_cast<Connection*>(uintptr_t{0x1000});
    Connection* b = reinterpret_cast<Connection*>(uintptr_t{0x2000});

    TopicRegistry reg;

    // 订阅两个连接到同一个 topic
    reg.subscribe("imu", a);
    reg.subscribe("imu", b);
    auto subs = reg.getSubscribers("imu");
    if (subs.size() == 2) {
        printf("OK: 订阅两个连接 -> getSubscribers(imu) 返回 2 个\n");
    } else {
        printf("FAIL: 期望 2 个，实际 %zu 个\n", subs.size());
        return 1;
    }

    // 同一个连接重复订阅，set 自动去重
    reg.subscribe("imu", a);
    subs = reg.getSubscribers("imu");
    if (subs.size() == 2) {
        printf("OK: 重复订阅 a，set 去重后仍是 2 个\n");
    } else {
        printf("FAIL: 重复订阅后应去重为 2 个，实际 %zu\n", subs.size());
        return 1;
    }

    // a 还订阅了 camera
    reg.subscribe("camera", a);
    if (reg.getSubscribers("camera").size() == 1) {
        printf("OK: a 订阅 camera -> getSubscribers(camera) 返回 1 个\n");
    } else {
        printf("FAIL: camera 应有 1 个订阅者\n");
        return 1;
    }

    // removeConnection(a)：一次把 a 从所有 topic 剔掉
    reg.removeConnection(a);
    if (reg.getSubscribers("imu").size() == 1 &&
        reg.getSubscribers("camera").empty()) {
        printf("OK: removeConnection(a) 后 imu 剩 1 个、camera 清空\n");
    } else {
        printf("FAIL: removeConnection(a) 没清干净\n");
        return 1;
    }

    // removeConnection(b)：清光 imu
    reg.removeConnection(b);
    if (reg.getSubscribers("imu").empty()) {
        printf("OK: removeConnection(b) 后 imu 清空\n");
    } else {
        printf("FAIL: imu 应已清空\n");
        return 1;
    }

    // ⑥ unsubscribe 单独退订一个 topic
    reg.subscribe("lidar", a);
    reg.unsubscribe("lidar", a);
    if (reg.getSubscribers("lidar").empty()) {
        printf("OK: unsubscribe(lidar, a) 后 lidar 清空\n");
    } else {
        printf("FAIL: lidar 应已退订\n");
        return 1;
    }

    
    return 0;
}
