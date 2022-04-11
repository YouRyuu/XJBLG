#include "ChatServer.h"
#include "Eventloop.h"
#include "ChatContext.h"
#include "Buffer.h"

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name)
    : tcpServer_(loop, listenAddr, name)
{
    tcpServer_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    tcpServer_.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection, new conn [%s]\n", conn->name().c_str());
    }
    else
    {
        printf("onConnection, conn [%s] is down\n", conn->name().c_str());
        MutexLockGuard lock(mutex);
        userConnMaps_.erase(conn); //断开连接，把他从在线用户中删除
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n)
{
    //在这里，调用消息解码器，验证消息的合法性
    //如果消息合法，得到消息中的类型、发信人、收信人等等
    std::cout << buf->getAllStringFromBuffer() << std::endl;
    if (chatContext_.parse(buf)) //消息解析成功
    {
        //std::cout<<"After parse:"<<buf->getAllStringFromBuffer()<<std::endl;
        //开始处理请求
        onRequest(conn, chatContext_);
        chatContext_.reset();
    }
    //等待消息格式正确
    else
    {
        //下面这行是用来debug的
        //buf->getAllStringFromBuffer();
    }
}

void ChatServer::onRequest(const TcpConnectionPtr &conn, ChatContext &chatContext)
{
    int32_t length = chatContext_.getLength();
    std::string type = chatContext_.getType();
    std::string sender = chatContext_.getSender();
    std::string recver = chatContext_.getRevcer();
    std::string time = chatContext_.getTime();
    std::string body = chatContext_.getBody();
    if (type == "L")
    {
        //先验证用户名和密码是否正确
        // vaild(sender, body);
        bool vaild = true;
        if (vaild)
        {
            //验证成功
            //这里要先判断该用户是否已经在线，如果在线的话就把旧的连接踢出去，加入新的连接
            //如果conn已经存在userConnMaps中，说明不可能是登录消息
            TcpConnectionPtr preConn;
            int exists = 0;
            {
                MutexLockGuard lock(mutex);
                for (std::map<TcpConnectionPtr, std::string>::iterator it = userConnMaps_.begin(); it != userConnMaps_.end(); ++it)
                {
                    if (it->second == sender)
                    {
                        //用户已经在线了
                        exists = 1;
                        //把旧连接踢出去
                        preConn=it->first;
                        userConnMaps_.erase(it);
                        break;
                    }
                }
                userConnMaps_.insert({conn, sender}); //更新在线列表
            }
            if (exists) //用户是被挤出去的
            {
                std::cout<<"原用户被T"<<std::endl;
                //这里要给原客户发一条消息，提示它被挤出去了
                preConn->shutdown();
                preConn.reset();
            }
            //给客户端发消息说登录成功
            //消息格式：类型=M，代表系统消息， sender=10000， recver=发消息的人， 正文=状态码成功(200)
            Buffer message;
            std::string context = std::string("M") + std::string("1000") + sender + time + std::string("200");
            message.append(context);
            int32_t len = static_cast<int32_t>(context.size());
            int32_t be32 = hostToNetwork32(len);
            message.prepend(&be32, sizeof(be32));
            conn->send(&message);
        }
        else
        {
            //验证失败
            //给客户端发消息说登录失败
            Buffer message;
            std::string context = std::string("M") + std::string("1000") + sender + time + std::string("300");
            message.append(context);
            int32_t len = static_cast<int32_t>(context.size());
            int32_t be32 = hostToNetwork32(len);
            message.prepend(&be32, sizeof(be32));
            conn->send(&message);
        }
    }
    else if (type == "S")
    {
        //一个客户给另一个客户发了一条消息，这时候要根据recver的id在userConnMaps中找到接收者的conn，转发给它
        int count = 0;
        TcpConnectionPtr recverConn;
        {
            MutexLockGuard lock(mutex);
            for (std::map<TcpConnectionPtr, std::string>::iterator it = userConnMaps_.begin(); it != userConnMaps_.end(); ++it)
            {
                if (it->second == recver)
                {
                    count = 1;
                    recverConn = it->first;
                    break;
                }
            }
        }
        if (count) //该用户在线
        {
            Buffer message;
            std::string context = type + sender + recver + time + body;
            message.append(context);
            int32_t len = static_cast<int32_t>(context.size());
            int32_t be32 = hostToNetwork32(len);
            message.prepend(&be32, sizeof(be32));
            recverConn->send(&message);
        }
        else
        {
            //该用户不在线，把消息缓存起来
            std::cout << recver << "不在线" << std::endl;
        }
    }
    else
    {
        std::cout << "error message" << std::endl;
    }
}
