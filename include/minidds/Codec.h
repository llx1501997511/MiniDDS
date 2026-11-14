#ifndef MINIDDS_CODEC_H
#define MINIDDS_CODEC_H
#include <string>
#include <cstdint>
#include <vector>

namespace minidds{

class Codec{
public:
    //payload打包成带长度前缀的一个整帧
    static std::string encode(const std::string& payload);
    //返回切出的所有完整帧。半包的时候什么都不返回，等下次再喂。
    static std::vector<std::string> decode(std::string& buffer);

    //长度占4个字节
    static const size_t kHeaderLen=4;


};


}

#endif