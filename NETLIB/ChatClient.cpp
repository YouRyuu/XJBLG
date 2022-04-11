#include "ChatClient.h"
#include "Eventloop.h"
#include "TcpClient.h"

ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg)
    : client_(loop, serverAddr, "ChatClient")
{
    client_.setConnectionCallback(std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallback(std::bind(&ChatClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
ChatClient::~ChatClient()
{
}
void ChatClient::connect()
{
    client_.connect();
}
void ChatClient::disconnect()
{
    client_.disconnect();
}

void ChatClient::onConnection(const TcpConnectionPtr &conn)
{
    //连接建立成功后会调用这个函数，即在这个函数里实现登录数据传输到server
    if (conn->connected()) //连接建立成功
    {
        connection_ = conn;
        //给服务端发登录消息
        //消息格式：类型=L，代表登录消息， sender=用户id， recver=10000， 正文=password
        Buffer message;
        std::string context = std::string("L") + userId_ + std::string("1000") + std::string("99999999") + std::string("password");
        message.append(context);
        int32_t len = static_cast<int32_t>(context.size());
        int32_t be32 = hostToNetwork32(len);
        message.prepend(&be32, sizeof(be32));
        conn->send(&message);
    }
    else //连接建立失败
    {
        std::cout << "连接建立失败" << std::endl;
        connection_.reset();
    }
}
void ChatClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n)
{
    //对收到的消息进行解码
    if(chatContext_.parse(buf))
    {
        std::string type = chatContext_.getType();
        std::string sender = chatContext_.getSender();
        std::string recver = chatContext_.getRevcer();
        std::string time = chatContext_.getTime();
        std::string body = chatContext_.getBody();
        if(type=="S")
        {
            printf("用户%s于%s给您发来的消息:%s\n", sender.c_str(), time.c_str(), body.c_str());
        }
        else if(type=="M")
        {
            //系统消息
        }
    }
}

void ChatClient::send(std::string &line)
{
    Buffer message;
    std::string context = std::string("S") + userId_ + std::string("2222") + std::string("99999999") + line;
    message.append(context);
    int32_t len = static_cast<int32_t>(context.size());
    int32_t be32 = hostToNetwork32(len);
    message.prepend(&be32, sizeof(be32));
    connection_->send(&message);
}