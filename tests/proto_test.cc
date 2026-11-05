// Day 4 验证：protobuf 序列化 + oneof 判断消息类型
#include "message.pb.h"
#include <cstdio>
#include <string>

using minidds::Control;
using minidds::Envelope;
using minidds::Message;

int main() {
    // 造一个装「数据消息」的信封
    Envelope env;
    Message* m = env.mutable_message();   // mutable_xxx()：拿 message 字段的可变指针，往里填
    m->set_topic("camera");
    m->set_seq(1);
    m->set_payload("hello-dds");

    // 序列化成二进制字节串
    std::string bytes;
    if (!env.SerializeToString(&bytes)) {
        printf("FAIL: 序列化失败\n");
        return 1;
    }
    printf("序列化成功，字节数 = %zu\n", bytes.size());

    // 反序列化回一个全新的信封（模拟「对面收到这串字节」）
    Envelope env2;
    if (!env2.ParseFromString(bytes)) {
        printf("FAIL: 反序列化失败\n");
        return 1;
    }

    // oneof 判断：这一帧装的是 message 还是 control？
    if (env2.has_message()) {
        const Message& m2 = env2.message();
        printf("OK: has_message() = true\n");
        printf("    topic = %s, seq = %llu, payload = %s\n",
               m2.topic().c_str(),
               (unsigned long long)m2.seq(),
               m2.payload().c_str());
    } else {
        printf("FAIL: 应该是数据消息，却判成了控制消息\n");
        return 1;
    }

    // 再验证装「控制消息」的信封（订阅）
    Envelope env3;
    env3.mutable_control()->set_type(Control::SUB);
    env3.mutable_control()->set_topic("camera");

    std::string bytes3;
    env3.SerializeToString(&bytes3);

    Envelope env4;
    env4.ParseFromString(bytes3);
    if (env4.has_control() && !env4.has_message()) {
        printf("OK: has_control() = true, type = %d, topic = %s\n",
               (int)env4.control().type(),
               env4.control().topic().c_str());
    } else {
        printf("FAIL: 控制消息往返失败\n");
        return 1;
    }

    // 关键验证：一段乱字节，oneof 不会误判
    Envelope env5;
    std::string garbage = "xxxx";   // 不是合法信封
    env5.ParseFromString(garbage);
    if (!env5.has_control() && !env5.has_message()) {
        printf("OK: 乱字节既不是 control 也不是 message（oneof 可靠）\n");
    } else {
        printf("FAIL: 乱字节被误判了\n");
        return 1;
    }

    printf("ALL PASS\n");
    return 0;
}
