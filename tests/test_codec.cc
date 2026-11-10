//Day9 Codec单元测试
#include "minidds/Codec.h"
#include <cassert>
#include <iostream>

int main(){
    using minidds::Codec;
    //1，单帧往返
    std::string f1=Codec::encode("hello");
    std::string buf=f1;
    auto frames=Codec::decode(buf);
    assert(frames.size()==1&& frames[0]=="hello");

    //2.半包，一条消息分两半喂
    std::string full=Codec::encode("abcdef");
    std::string half1=full.substr(0,5);
    std::string half2=full.substr(5);
    auto r1=Codec::decode(half1);
    assert(r1.empty());
    half1+=half2;
    auto r2=Codec::decode(half1);
    assert(r2.size()&&r2[0]=="abcdef");

    //3，粘包，两条消息连在一起发
    std::string two=Codec::encode("aaa")+Codec::encode("bbb");
    auto r3=Codec::decode(two);
    assert(r3.size()==2 &&r3[0]=="aaa"&&r3[1]=="bbb");

    //4,长度异常，大于64M
    char bad[4]={0x7f,(char)0xff,(char)0xff,(char)0xff};
    std::string evil(bad,4);
    evil+="xx";
    auto r4=Codec::decode(evil);
    assert(r4.empty()&& evil.empty());

    std::cout<<"all codec tests passed\n";
    return 0;

}