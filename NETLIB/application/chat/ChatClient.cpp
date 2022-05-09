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
        sendMessage("1001", userId_, "10000000", getNowTime(), password_);
    }
    else //连接建立失败
    {
        std::cout << "连接建立失败" << std::endl;
        connection_.reset();
    }
}

void ChatClient::onContextMessage(const TcpConnectionPtr conn, ChatContext chatContext, int n)
{
    std::string code = chatContext.getCode();
    std::string seq = chatContext.getSeq();
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
        std::cout<<body.c_str()<<std::endl;
    }
    else if(code=="1020")
    {
        printf("好友列表:%s\n", body.c_str());
    }
    else
    {
        std::cout<<code<<seq<<sender<<recver<<time<<body<<std::endl;
    }
    sendMessage("1111", userId_, "10000000", getNowTime(), seq);
}

std::string ChatClient::encodeMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    //---------------------------//
    //{"code":"1111","sender":"1122","recver":"3344","time":"222222222","body":"xxxxx"}
    std::string seq = getSeq(); //获取一个随机的序列号
    std::string msg("{\"code\":\"");
    msg = msg + code;
    msg = msg + "\",\"seq\":\"";
    msg = msg + seq;
    msg = msg + "\",\"sender\":\"";
    msg = msg + sender;
    msg = msg + "\",\"recver\":\"";
    msg = msg + recver;
    msg = msg + "\",\"time\":\"";
    msg = msg + time;
    msg = msg + "\",\"body\":\"";
    msg = msg + body;
    msg = msg +"\"}";
    std::cout<<msg<<std::endl;
    return msg;
}

void ChatClient::sendMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    Buffer message;
    //std::string context = code + sender + recver + time + body;
    std::string context = encodeMessage(code, sender, recver, time, body);
    message.append(context);
    int32_t len = static_cast<int32_t>(context.size());
    int32_t be32 = hostToNetwork32(len);
    message.prepend(&be32, sizeof(be32));
    connection_->send(&message);
}

void ChatClient::send(std::string &line) //用于发送来自stdin的
{
    if(line[0]=='l')
    {
        //请求好友列表
        sendMessage("1002", userId_, "10000000", getNowTime(), line);
    }
    else if(line[0]=='a')
    {   //a 添加好友
        std::string body = line.substr(1);
        sendMessage("1005", userId_, "10000000", getNowTime(), body);
    }
    else if(line[0]=='d')
    {
        //删除好友
        std::string body = line.substr(1);
        sendMessage("1010", userId_, "10000000", getNowTime(), body);
    }
    else if(line[0]=='u')
    {
        //获取用户资料
        std::string body = line.substr(1);
        sendMessage("1006", userId_, "10000000", getNowTime(), body);
    }
    else if(line[0]=='g')
    {
        //同意好友申请
        std::string body = line.substr(1);
        sendMessage("1007", userId_, "10000000", getNowTime(), body);
    }
    else if(line[0]=='r')
    {
        //拒绝好友申请
        std::string body = line.substr(1);
        sendMessage("1008", userId_, "10000000", getNowTime(), body);
    }
    else if(line[0]=='z')
    {
        //查看好友申请 
        std::string body = line.substr(1);
        sendMessage("1009", userId_, "10000000", getNowTime(), body);
    }
    else
        sendMessage("2001", userId_, "22222222", getNowTime(), line);     //发消息
}

void ChatClient::sendMessageTo(std::string message, std::string recver)
{
    //发送消息给好友
    sendMessage("2001", userId_, recver, getNowTime(), message);
}