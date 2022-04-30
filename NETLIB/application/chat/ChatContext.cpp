#include "ChatContext.h"
#include "../../net/Buffer.h"
#include "../../net/Endian.h"
#include "../../net/TcpConnection.h"

void ChatContext::onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n)
{
    //std::cout<<buf->getAllStringFromBuffer()<<std::endl;
    while(buf->readableBytes() >= headerLength_)      //确保收到的消息是完整的，否则使用状态机
    {
        const void *data = buf->peek();
        int32_t be32 = *(static_cast<const int32_t*>(data));
        length = networkToHost32(be32);
        if(length > 65536 || length < 0)
        {
            std::cout<<"ChatContext::parse: error length"<<std::endl;
            conn->shutdown();
            break;
        }
        else if(buf->readableBytes()>=headerLength_ + length)     //收到了完整的消息
        {
            buf->retrieve(headerLength_);     //将头部表示长度的字节读出来
            // code_ = std::string(buf->peek(), codeLen);    //获取消息类型
            // buf->retrieve(codeLen);
            // sender_ = std::string(buf->peek(), senderLen);
            // buf->retrieve(senderLen);
            // recver_ = std::string(buf->peek(), recverLen);
            // buf->retrieve(recverLen);
            // time_ = std::string(buf->peek(), timeLen);
            // buf->retrieve(timeLen);
            // body_ = std::string(buf->peek(), length-codeLen-senderLen-recverLen-timeLen);
            // buf->retrieve(length-codeLen-senderLen-recverLen-timeLen);
            std::string msg = std::string(buf->peek(), length);
            std::cout<<"msg:"<<msg<<std::endl;
            buf->retrieve(length);
            bool succParse = jsonItem_.parseJson(&*msg.begin(), &*msg.end());
            if(succParse)
            {
                code_ = jsonItem_.jsonItem["code"];
                sender_ = jsonItem_.jsonItem["sender"];
                recver_ = jsonItem_.jsonItem["recver"];
                time_ = jsonItem_.jsonItem["time"];
                body_ = jsonItem_.jsonItem["body"];
                contextMessageCallback(conn, *this, 1);
            }
            else
            {
                //contextMessageCallback(conn, *this, 0);     //解析失败，错误的请求
                conn->shutdown();       //4.30:等待测试这个，没有解析成功会不会断开连接
            }
            //contextMessageCallback(conn, *this, n);
        }
        else
        {
            break;
        }
    }
}