#ifndef CHATSERVER_H
#define CHATSERVER_H

//聊天服务器
/*
服务器端需要做的工作：
    解码消息：
        消息类型：
            注册、登录、一个客户发送给另一个客户的消息、加好友、删好友、查找用户信息
            当一个新的连接到来时（OnConnection），它是登录消息（暂时不考虑注册），这时候需要维护一个在线用户和它的conn对应关系
            因为客户给另一个客户发消息是通过server中转的，server需要知道用户ID
*/
#include "../../net/Mutex.h"
#include "../../net/Callbacks.h"
#include "ChatContext.h"
#include "../../net/TcpServer.h"
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include "ChatModel.h"

class EventLoop;
class TcpServer;
class InetAddress;
class Buffer;
class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr, const std::string &name);

    void setThreadNum(int numThreads)
    {
        tcpServer_.setThreadNum(numThreads);
    }

    void start()
    {
        tcpServer_.start();
    }
    void sendMessage(const TcpConnectionPtr &conn, std::string msg);
    void sendMessage(const TcpConnectionPtr &conn, std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList=false);
    void cacheMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body);
    bool getUserState(std::string userId);
    std::vector<std::pair<std::string, std::string>> getUserFriends(std::string userId);
    std::string userFriendsToString(std::vector<std::pair<std::string, std::string>> /*&*/ friends);
    std::string getUserInfo(std::string userID);
    FriendState checkIsFriend(std::string user1ID, std::string user2ID);
    std::string encodeMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList);
    bool agreeRequestOfAddFriend(std::string userID, std::string applyID);
    void doLoginAction(const TcpConnectionPtr &conn, std::string userID, std::string password);
    void doGetFriendsListAction(const TcpConnectionPtr &conn, std::string userID);
    void doAddFriendAction(const TcpConnectionPtr &conn, std::string userID, std::string addedID);
    void doGetUserMessageAction(const TcpConnectionPtr &conn, std::string userID, std::string searchID);
    void doAgreeReqOfAddFriend(const TcpConnectionPtr &conn, std::string userID, std::string applyID);
    void doRefuseReqOfAddFriend(const TcpConnectionPtr &conn, std::string userID, std::string applyID);
    void doSendMessageAction(const TcpConnectionPtr &conn, std::string code, std::string sendID, std::string recvID, std::string sendTime, std::string message);
    void doGetMyFriendReqAction(const TcpConnectionPtr &conn, std::string userID);
    void doOfflineNotifyAction(std::string userID);
    void doOnlineNotifyAction(std::string userID);
    void doDeleteFriendAction(const TcpConnectionPtr &conn, std::string userID, std::string deleteID);
    void sendOfflineMessageAction(const TcpConnectionPtr &conn, std::string userID);
    std::string getNowTime();
private:
    void onConnection(const TcpConnectionPtr &conn);
    void onContextMessage(const TcpConnectionPtr &conn, ChatContext chatContext, int n);
    void onRequest(const TcpConnectionPtr &conn, ChatContext &chatContext);
    bool loginValid(const std::string userid, const std::string password);
    bool checkUserIsExist(std::string userID);
    const static std::string systemId;
    TcpServer tcpServer_;
    typedef std::map<std::string, TcpConnectionPtr> UserConnMap;
    typedef std::map<std::string, std::vector<std::string>> CachedMessages; //缓存的消息
    UserConnMap userConnMaps_;
    CachedMessages cachedMessages_;
    MutexLock mutex;
    ChatContext chatContext_;
    UserModel userModel_;
    JsonItem jsonItem_;
};

#endif