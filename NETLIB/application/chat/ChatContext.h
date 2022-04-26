#ifndef CHATCONTEXT_H
#define CHATCONTEXT_H

#include <string.h>
#include <string>
#include "../../net/Callbacks.h"

class Buffer;
class ChatContext
{
    //一个聊天程序的消息处理
    /*消息格式：
     *                      长度  状态码4字节    8字节    8字节   8字节 
     *  00 00 00 00 00 00 00 00     code      sender  recver  time  body
     * ｜    prepend          ｜
     * 状态码：1开头的代表系统和用户之间直接交互消息，2开头的代表用户之间的消息
     * 1000:操作成功
     * 1001:请求登录  1002:请求好友列表  1003:搜索用户  1004:删除好友  1005:添加好友
     * 1010:登录成功  1011:登录失败（密码错误） 1012:被挤占下线   1013:非好友关系
     * 1020:返回好友列表  
     * 猜想：这里客户端对服务器返回的消息进行处理时：
     *     如果客户端的请求是正确的，就返回1000，那么客户端怎么知道这个1000是对哪个请求的反馈呢？
     *     客户端对它发出的每个请求维护一个请求队列，每个请求都会有一个序列号（这里用time字段代替），在服务器返回对客户端的响应的时候
     *     对于的序列号字段就是某个请求的序列号，客户端在收到回馈之后，在队列里找到这个请求，将这个请求出队，然后执行相应的回调函数。
     * 2001:用户A给用户B发消息
     */
    public:
        typedef std::function<void(const TcpConnectionPtr&, ChatContext, int)> ContextMessageCallback;
        ChatContext()/*:state_(TYPE)*/
        {   }

        ChatContext(const ContextMessageCallback &cb)
        :contextMessageCallback(cb)
        {
        }

        bool parse(Buffer *buf);

        void onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n);

        void reset()
        {
            length = 0;
            code_ = "";
            sender_ = "";
            recver_ = "";
            time_ = "";
            body_ = "";
        }

        std::string getCode()
        {
            return code_;
        }

        std::string getSender()
        {
            return sender_;
        }

        std::string getRevcer()
        {
            return recver_;
        }

        std::string getTime()
        {
            return time_;
        }

        std::string getBody()
        {
            return body_;
        }

        int32_t getLength()
        {
            return length;
        }

        void setContextMessageCallback(const ContextMessageCallback &cb)
        {
            contextMessageCallback = cb;
        }

    private:
        //MessageParseState state_;
        const static size_t headerLength_ = sizeof(int32_t);     //头部长度4字节
        const static int codeLen = 4;
        const static int senderLen = 8;
        const static int recverLen = 8;
        const static int timeLen = 8;
        int32_t length;
        std::string code_;          //状态码
        std::string sender_;        //发送消息的人
        std::string recver_;        //收消息的人
        std::string time_;          //发送时间戳
        std::string body_;          //消息正文
        ContextMessageCallback contextMessageCallback;
};

#endif