#include "minidds/Server.h"
#include "minidds/EventLoop.h"
#include <cstdio>
#include <unistd.h>

int main(int argc,char* argv[]){
    setvbuf(stdout,NULL,_IONBF,0);
    printf("[IO线程] tid=%ld\n",(long)gettid());
    int port=8000;
    if(argc>1){
        port=atoi(argv[1]);
    }

    minidds::EventLoop loop;
    minidds::Server server(&loop,port);
    server.start();

    loop.loop();
    return 0;
    
}