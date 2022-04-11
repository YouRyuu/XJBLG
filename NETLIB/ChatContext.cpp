#include "ChatContext.h"
#include "Buffer.h"
#include "Endian.h"

bool ChatContext::parse(Buffer *buf)
{
    while(buf->readableBytes() >= headLength_)      //确保收到的消息是完整的，否则使用状态机
    {
        const void *data = buf->peek();
        int32_t be32 = *(static_cast<const int32_t*>(data));
        length = networkToHost32(be32);
        if(length > 65536 || length < 0)
        {
            std::cout<<"ChatContext::parse: error length"<<std::endl;
            return false;
        }
        else if(buf->readableBytes()>=headLength_ + length)     //收到了完整的消息
        {
            buf->retrieve(headLength_);     //将头部表示长度的字节读出来
            type_ = std::string(buf->peek(), 1);    //获取消息类型
            buf->retrieve(1);
            sender_ = std::string(buf->peek(), 4);
            buf->retrieve(4);
            recver_ = std::string(buf->peek(), 4);
            buf->retrieve(4);
            time_ = std::string(buf->peek(), 8);
            buf->retrieve(8);
            //body_ = buf->retrieveAllAsString();
            body_ = std::string(buf->peek(), length-17);
            buf->retrieve(length-17);
            return true;
        }
        else
        {
            return false;      //消息不完整，继续接收直到完整
        }
    }
}