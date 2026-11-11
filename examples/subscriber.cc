// 用法: ./subscriber [端口] [topic]

#include <minidds/Codec.h>
#include <message.pb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

int main(int argc,char* argv[]){
    setvbuf(stdout,NULL,_IONBF,0);
    int port=8000;
    const char* topic="imu";
    if(argc>1) port=atoi(argv[1]);
    if(argc>2) topic=argv[2];

    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);

    if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("connect");
        return 1;
    }

    minidds::Envelope env;
    auto* ctrl=env.mutable_control();
    ctrl->set_type(minidds::Control::SUB);
    ctrl->set_topic(topic);
    
    std::string payload;
    env.SerializeToString(&payload);
    std::string frame=minidds::Codec::encode(payload);
    send(fd,frame.data(),frame.size(),MSG_NOSIGNAL);
    printf("subscribed %s\n",topic);

    std::string buf;
    char tmp[65536];
    while(true){
        ssize_t n=recv(fd,tmp,sizeof(tmp),0);
        if(n<=0)    break;
        buf.append(tmp,n);

        auto frames=minidds::Codec::decode(buf);
        for(auto& f:frames){
            minidds::Envelope env;
            if(env.ParseFromString(f)&&env.has_message()){
                const auto& msg=env.message();
                //printf("recv topic=%s seq=%lu payload=%s\n",msg.topic().c_str()
                //    ,(unsigned long)msg.seq(),msg.payload().c_str());
                //测试短写大包，直接打数据，不输出内容。
                printf("recv topic=%s seq=%lu len=%zu\n",msg.topic().c_str()
                    ,(unsigned long)msg.seq(),msg.payload().size());
            }
        }
    }
    close(fd);
    return 0;

}