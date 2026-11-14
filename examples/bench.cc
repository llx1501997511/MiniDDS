// 用法: ./bench [端口] [条数] [topic]
#include <minidds/Codec.h>
#include <message.pb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>


int main(int argc,char* argv[]){
    int port=8000;
    int count=100000;
    const char* topic="bench";
    if(argc>1) port=atoi(argv[1]);
    if(argc>2) count=atoi(argv[2]);
    if(argc>3) topic=argv[3];
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0){ perror("socket"); return 1; }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);
    if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("connect");
        return 1;
    }
    long long sent_bytes=0;
    int fail=0;
    auto t0=std::chrono::steady_clock::now();
    for(int i=0;i<count;i++){
        minidds::Envelope env;
        auto* msg=env.mutable_message();
        msg->set_topic(topic);
        msg->set_seq(i);
        msg->set_payload("x");
        std::string payload;
        env.SerializeToString(&payload);
        std::string frame=minidds::Codec::encode(payload);
        ssize_t n=send(fd,frame.data(),frame.size(),MSG_NOSIGNAL);
        if(n<0){ fail++; continue; }
        sent_bytes+=n;
    }
    auto t1=std::chrono::steady_clock::now();
    double sec=std::chrono::duration<double>(t1-t0).count();
    int ok=count-fail;
    printf("%d 条 / %.3f 秒, %lld 字节, 失败 %d\n",ok,sec,sent_bytes,fail);
    printf("QPS %d, 平均每条 %.2f 微秒\n",(int)(ok/sec),sec*1e6/ok);
    close(fd);
    return 0;
}