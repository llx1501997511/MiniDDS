#include "minidds/Codec.h"
#include <arpa/inet.h>
#include <cstring>
namespace minidds {

std::string Codec::encode(const std::string& payload){
    uint32_t len=static_cast<uint32_t>(payload.size());
    uint32_t netLen=htonl(len);

    std::string frame;
    frame.reserve(kHeaderLen+payload.size());
    frame.append(reinterpret_cast<const char*>(&netLen),kHeaderLen);
    frame.append(payload);
    return frame;
}

std::vector<std::string> Codec::decode(std::string& buffer){
    std::vector<std::string> frames;
    size_t offset=0;

    while(buffer.size()-offset>=kHeaderLen){
        uint32_t netLen=0;
        memcpy(&netLen,buffer.data()+offset,kHeaderLen);
        uint32_t len=ntohl(netLen);

        if(len>64*1024*1024){
            buffer.clear();
            return frames;
        }

        if(buffer.size()-offset<kHeaderLen+len){
            break;
        }

        frames.emplace_back(buffer,offset+kHeaderLen,len);
        offset+=kHeaderLen+len;

    }
    if(offset>0){
        buffer.erase(0,offset);
    }
    return frames;
}

} // namespace