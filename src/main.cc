#include "minidds/Server.h"
#include "minidds/EventLoop.h"
#include <cstdio>
#include <unistd.h>
#include <csignal>

static minidds::EventLoop* g_loop=nullptr;

static void onSigint(int){
    if(g_loop) g_loop->quit();
}

int main(int argc,char* argv[]){
    setvbuf(stdout,NULL,_IONBF,0);
    printf("[IO线程] tid=%ld\n",(long)gettid());
    int port=8000;
    if(argc>1){
        port=atoi(argv[1]);
    }

    minidds::EventLoop loop;
    g_loop=&loop;
    signal(SIGINT,onSigint);

    minidds::Server server(&loop,port);
    if(!server.start()){
        fprintf(stderr,"server 启动失败, 端口 %d 可能被占用\n",port);
        return 1;
    }

    loop.loop();
    printf("Server 退出，开始析构\n");
    return 0;
    
}