#include "ChatClient.h"
#include "../../net/Eventloop.h"
//#include "../../net/TcpClient.h"

ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg)
    : client_(loop, serverAddr, "ChatClient"),
    chatContext_(std::bind(&ChatClient::onContextMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
    client_.setConnectionCallback(std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallback(std::bind(&ChatContext::onMessage, &chatContext_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
        sendMessage("1001", userId_, "10000000", "99999999", password_);
    }
    else //连接建立失败
    {
        std::cout << "连接建立失败" << std::endl;
        connection_.reset();
    }
}
void ChatClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n)
{
    std::cout << buf->getAllStringFromBuffer() << std::endl;
    //对收到的消息进行解码
    //使用while是因为如果一起发送多条消息的时候很可能合并到一起发送了，也就是说buf里的消息没有解析完，还有剩下的一条完整的消息
    //这是在处理发送缓存消息的时候发现的
    // while (buf->readableBytes() > 0)
    //{
    if (chatContext_.parse(buf))
    {
        std::string code = chatContext_.getCode();
        std::string sender = chatContext_.getSender();
        std::string recver = chatContext_.getRevcer();
        std::string time = chatContext_.getTime();
        std::string body = chatContext_.getBody();
        if (code == "2001")
        {
            printf("用户%s于%s给您发来的消息:%s\n", sender.c_str(), time.c_str(), body.c_str());
        }
        // else
        else if (code == "1000")
        {
            std::cout << "成功" << std::endl;
        }
    }
    //}
}

void ChatClient::onContextMessage(const TcpConnectionPtr conn, ChatContext chatContext, int n)
{
    std::string code = chatContext.getCode();
    std::string sender = chatContext.getSender();
    std::string recver = chatContext.getRevcer();
    std::string time = chatContext.getTime();
    std::string body = chatContext.getBody();
    if (code == "2001")
    {
        printf("用户%s于%s给您发来的消息:%s\n", sender.c_str(), time.c_str(), body.c_str());
    }
    // else
    else if (code == "1000")
    {
        std::cout << "成功" << std::endl;
    }
    else if(code=="1020")
    {
        printf("好友列表:%s\n", body.c_str());
    }
    else
    {
        std::cout<<code<<sender<<recver<<time<<body<<std::endl;
    }
}

void ChatClient::sendMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    Buffer message;
    std::string context = code + sender + recver + time + body;
    message.append(context);
    int32_t len = static_cast<int32_t>(context.size());
    int32_t be32 = hostToNetwork32(len);
    message.prepend(&be32, sizeof(be32));
    connection_->send(&message);
}

void ChatClient::send(std::string &line) //用于发送来自stdin的
{
    if(line[0]=='f')
    {
        sendMessage("1002", userId_, "10000000", "99999999", line);
    }
    else
        sendMessage("2001", userId_, "22222222", "99999999", line);
}

void ChatClient::sendMessageTo(std::string message, std::string recver)
{
    //发送消息给好友
    sendMessage("2001", userId_, recver, "99999999", message);
}