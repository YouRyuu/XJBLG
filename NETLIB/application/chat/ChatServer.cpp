#include "ChatServer.h"
#include "../../net/Eventloop.h"
#include "ChatContext.h"
#include "../../net/Buffer.h"
//#include "../../mysql/mysqlDB.h"


ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name)
    : tcpServer_(loop, listenAddr, name),
      chatContext_(std::bind(&ChatServer::onContextMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
    tcpServer_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    tcpServer_.setMessageCallback(std::bind(&ChatContext::onMessage, &chatContext_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())                                                                                                                                                                                                                                                                
    {
        printf("onConnection, new conn [%s]\n", conn->name().c_str());
    }
    else
    {
        //断开连接，把他从在线用户中删除
        //同时要通知他的所有在线好友，他下线了
        //先获取他的所有好友列表
        printf("onConnection, conn [%s] is down\n", conn->name().c_str());
        MutexLockGuard lock(mutex);
        for (std::map<std::string, TcpConnectionPtr>::iterator it = userConnMaps_.begin(); it != userConnMaps_.end(); ++it)
        {
            if (it->second == conn)
            {
                userConnMaps_.erase(it);
                break;
            }
        }
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, int n)
{
    //在这里，调用消息解码器，验证消息的合法性
    //如果消息合法，得到消息中的类型、发信人、收信人等等
    if (chatContext_.parse(buf)) //消息解析成功
    {
        //开始处理请求
        onRequest(conn, chatContext_);
        chatContext_.reset();
    }
    //等待消息格式正确
    else
    {
        //下面这行是用来debug的
        // buf->getAllStringFromBuffer();
    }
}

void ChatServer::onContextMessage(const TcpConnectionPtr conn, ChatContext chatContext, int n)
{
    onRequest(conn, chatContext);
}

void ChatServer::onRequest(const TcpConnectionPtr &conn, ChatContext &chatContext)
{
    int32_t length = chatContext_.getLength();
    std::string code = chatContext_.getCode();
    std::string sender = chatContext_.getSender();
    std::string recver = chatContext_.getRevcer();
    std::string time = chatContext_.getTime();
    std::string body = chatContext_.getBody();
    // todo：这里要改成策略模式
    if (code == "1001")
    {
        std::string userMsg = model_.validUser(sender, body);
        //bool vaild = loginValid(sender, body);
        if(userMsg.size() > 0)
        //if (vaild)
        {
            //验证成功
            //这里要先判断该用户是否已经在线，如果在线的话就把旧的连接踢出去，加入新的连接
            //如果conn已经存在userConnMaps中，说明不可能是登录消息
            if (getUserState(sender)) //用户已经在线
            {
                TcpConnectionPtr preConn;
                {
                    MutexLockGuard lock(mutex);
                    preConn = userConnMaps_[sender];
                    userConnMaps_.erase(sender); //删除旧连接，添加新连接
                }
                //这里要给原客户发一条消息，提示它被挤出去了
                sendMessage(preConn, "1012", recver, sender, time, "被挤占下线");
                preConn->shutdown();
                preConn.reset();
            }
            {
                MutexLockGuard lock(mutex);
                userConnMaps_.insert({sender, conn});
            }
            //给客户端发消息说登录成功
            //同时需要找到当用户不在线时，是否有别的用户给这位用户发消息，在这里把这些消息也要发送给他
            sendMessage(conn, "1000", recver, sender, time, userMsg+"登录成功");
            std::map<std::string, std::vector<std::string>>::iterator it;
            int count = 0;
            {
                MutexLockGuard lock(mutex);
                it = cachedMessages_.find(sender);
                if (it != cachedMessages_.end())
                    count = 1;
            }
            if (count) //有该用户的未收消息
            {
                for (auto &msg : it->second)
                {
                    sendMessage(conn, msg);
                }
                std::cout << "缓存消息发送完毕:" << std::endl;
                {
                    MutexLockGuard lock(mutex);
                    cachedMessages_.erase(it);
                }
            }
        }
        else
        {
            //验证失败
            //给客户端发消息说登录失败
            sendMessage(conn, "1011", recver, sender, time, "登录失败(用户名或密码错误)");
            conn->shutdown();
        }
    }
    else if (code == "1002")
    {
        //请求好友列表
        std::string friends = userFriendsToString(getUserFriends(sender));
        sendMessage(conn, "1020", recver, sender, time, friends);
    }
    else if(code=="1005")
    {
        //添加好友
        //sender=sender, recver=system, time=time, body=要寻找的那个人的id
        //先从数据库里面找到这个人，如果存在再从好友关系里面找到这个人
        
    }
    else if (code == "2001")
    {
        //一个客户给另一个客户发了一条消息
        //要先判断recver是不是sender的好友。如果是，如果在线，直接发送，如果不在线，缓存消息
        //如果不是好友关系，则返回发送失败
        bool isFriend = true;
        // isFriend = relationship(sender, recver);
        if (isFriend)       //存在好友关系
        {
            if (getUserState(recver)) //用户在线
            {
                TcpConnectionPtr recvConn = userConnMaps_[recver];
                sendMessage(recvConn, code, sender, recver, time, body);
            }
            else
            {
                //该用户不在线，把消息缓存起来
                cacheMessage(code, sender, recver, time, body);
                std::cout << recver << "不在线" << std::endl;
            }
        }
        else        //不存在好友关系
        {
            sendMessage(conn, "1013", std::to_string(systemId), sender, time, recver+"不是您的好友");
        }
    }
    else
    {
        //错误的消息，把conn踢出去
        conn->shutdown();
        std::cout << "error message" << std::endl;
    }
}

void ChatServer::sendMessage(const TcpConnectionPtr &conn, std::string msg)
{
    Buffer message;
    message.append(msg);
    int32_t len = static_cast<int32_t>(msg.size());
    int32_t be32 = hostToNetwork32(len);
    message.prepend(&be32, sizeof(be32));
    conn->send(&message);
}

void ChatServer::sendMessage(const TcpConnectionPtr &conn, std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    std::string context = code + sender + recver + time + body;
    sendMessage(conn, context);
}

void ChatServer::cacheMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    std::string message = code + sender + recver + time + body;
    {
        MutexLockGuard lock(mutex);
        std::map<std::string, std::vector<std::string>>::iterator it = cachedMessages_.find(recver);
        if (it != cachedMessages_.end()) //已经有了recver的缓存消息
        {
            it->second.push_back(message);
        }
        else //此前没有此用户的要接收消息
        {
            cachedMessages_.insert({recver, {message}});
        }
    }
}

bool ChatServer::getUserState(std::string userId)
{
    //参数：要查找的用户的id
    //根据用户id获取用户的在线状态
    //若在线返回true，反之返回false
    bool state = false;
    {
        MutexLockGuard lock(mutex);
        std::map<std::string, TcpConnectionPtr>::iterator it = userConnMaps_.find(userId);
        if (it != userConnMaps_.end())
            state = true;
        else
            state = false;
    }
    return state;
}

std::vector<std::pair<std::string, std::string>> ChatServer::getUserFriends(std::string userId)
{
    //获取用户的好友列表
    // userId:用户id
    // return:[pair<userid, username>]
    // db.getUserFriends(userId);
    //select userid, username from friends where user1=userId and user2=myid union select
    std::vector<std::pair<std::string, std::string>> rt;
    rt.push_back({"22222222", "苞米"});
    rt.push_back({"33333333", "高建"});
    rt.push_back({"44444444", "梁灿"});
    return rt;
}

std::string ChatServer::userFriendsToString(std::vector<std::pair<std::string, std::string>> /*&*/ friends)
{
    std::string fs;
    for (int i = 0; i < friends.size(); ++i)
    {
        fs = fs + "\"" + friends[i].first + "\":\"" + friends[i].second + "\",";
    }
    return fs;
}

// bool ChatServer::loginValid(const std::string userid, const std::string password)
// {
//     MysqlDB db;
//     MysqlDB::ResultSetPtr res;
//     std::string sql("select userid from users where userid=\"");
//     sql = sql + userid;
//     sql = sql + "\" and password=\"";
//     sql = sql + password + "\"";
//     res = db.execute(sql);
//     if(res->rowsCount()!=0)
//         return true;
//     return false;
// }
