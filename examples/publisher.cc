// 用法: ./publisher [端口] [topic] [条数]

#include <minidds/Codec.h>
#include <message.pb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

int main(int argc,char* argv[]){
    int port=8000;
    const char* topic="imu";
    int count=10;
    if(argc>1) port=atoi(argv[1]);
    if(argc>2) topic=argv[2];
    if(argc>3) count=atoi(argv[3]);

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

    for(int i=0;i<count;i++){
        minidds::Envelope env;
        auto* msg=env.mutable_message();
        msg->set_topic(topic);
        msg->set_seq(i);
        msg->set_payload("hello-"+std::to_string(i));
        //msg->set_payload(std::string(10*1024*1024,'A')); //10M的A，测短写

        std::string payload;
        env.SerializeToString(&payload);
        std::string frame = minidds::Codec::encode(payload);

        send(fd,frame.data(),frame.size(),MSG_NOSIGNAL);
        printf("publish #%d: %s\n",i,msg->payload().c_str());
        usleep(100*1000);//间隔100ms,进行观察

    }
    close(fd);
    return 0;

}