#ifndef CHATCONTEXT_H
#define CHATCONTEXT_H

#include <string.h>
#include <string>

class Buffer;
class ChatContext
{
    //一个聊天程序的消息处理
    /*消息格式：
     *                      长度   4字节    4字节   8字节
     *  00 00 00 00 00 00 00 00  sender  recver  time  body
     * ｜    prepend          ｜
     * 
     * 
     * 
     * 
     */
    public:
        // enum MessageParseState
        // {
        //     TYPE, SENDER, RECVER, TIME, BODY, OVER
        // };

        ChatContext()/*:state_(TYPE)*/
        {   }

        bool parse(Buffer *buf);

        // bool over()
        // {
        //     return state_ == OVER;
        // }

        void reset()
        {
            //state_ = TYPE;
            //headLength_ = 0;
            length = 0;
            type_ = ' ';
            sender_ = "";
            recver_ = "";
            time_ = "";
            body_ = "";
        }

        std::string getType()
        {
            return type_;
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

    private:
        //MessageParseState state_;
        const static size_t headLength_ = sizeof(int32_t);     //头部长度
        int32_t length;
        std::string type_;     //消息类型  M:系统消息，L:登录， S:一个客户给另一个客户发消息...
        std::string sender_;        //发送消息的人
        std::string recver_;        //收消息的人
        std::string time_;          //发送时间戳
        std::string body_;              //消息正文
};

#endif