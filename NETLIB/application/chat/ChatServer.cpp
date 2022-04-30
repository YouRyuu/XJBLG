#include "ChatServer.h"
#include "../../net/Eventloop.h"
#include "../../net/Buffer.h"
#include <ctime>

const std::string ChatServer::systemId = "10000000";

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
        std::string userID;
        {
            MutexLockGuard lock(mutex);
            for (std::map<std::string, TcpConnectionPtr>::iterator it = userConnMaps_.begin(); it != userConnMaps_.end(); ++it)
            {
                if (it->second == conn)
                {
                    userID = it->first;
                    userConnMaps_.erase(it);
                    break;
                }
            }
        }
        if (userID.size() > 0)      //因为如果用户是被挤下去的话，在登录处理里面这个连接就会被删掉，此时获取不到userID。
            doOfflineNotifyAction(userID);
    }
}

void ChatServer::onContextMessage(const TcpConnectionPtr &conn, ChatContext chatContext, int n)
{
    //if(n)       //正确的请求
        onRequest(conn, chatContext);
   // else        //错误的请求
   // {
   //     conn->shutdown();
   // }
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
        doLoginAction(conn, sender, body);
    }
    else if (code == "1002")
    {
        //请求好友列表
        doGetFriendsListAction(conn, sender);
    }
    else if (code == "1005")
    {
        //添加好友
        doAddFriendAction(conn, sender, body);
    }
    else if (code == "1006")
    {
        //查看用户信息
        doGetUserMessageAction(conn, sender, body);
    }
    else if (code == "1007")
    {
        //同意好友申请
        doAgreeReqOfAddFriend(conn, sender, body);
    }
    else if (code == "1008")
    {
        //拒绝好友申请
        doRefuseReqOfAddFriend(conn, sender, body);
    }
    else if(code=="1009")
    {
        //查看好友申请
        doGetMyFriendReqAction(conn, sender);
    }
    else if (code == "2001")
    {
        doSendMessageAction(conn, code, sender, recver, time, body);
    }
    else
    {
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

std::string ChatServer::encodeMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList)
{
    //---------------------------//
    //{"code":"1111","sender":"1122","recver":"3344","time":"222222222","body":"xxxxx"}
    //{"code":"1","userid":"11111111","username":"bm"}
    std::string msg("{\"code\":\"");
    msg = msg + code;
    msg = msg + "\",\"sender\":\"";
    msg = msg + sender;
    msg = msg + "\",\"recver\":\"";
    msg = msg + recver;
    msg = msg + "\",\"time\":\"";
    msg = msg + time;
    if (!bodyIsList)
    {
        msg = msg + "\",\"body\":\"";
        msg = msg + body;
        msg = msg + "\"}";
    }
    else
    {
        msg = msg + "\",\"body\":";
        msg = msg + body;
        msg = msg + "}";
    }

    return msg;
}

void ChatServer::sendMessage(const TcpConnectionPtr &conn, std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList)
{
    std::string context = encodeMessage(code, sender, recver, time, body, bodyIsList);
    sendMessage(conn, context);
}

void ChatServer::cacheMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body)
{
    std::string message = encodeMessage(code, sender, recver, time, body, false);
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

std::vector<std::pair<std::string, std::string>> ChatServer::getUserFriends(std::string userID)
{
    //获取用户的好友列表
    // userId:用户id
    // return:[pair<userid, username>]
    // db.getUserFriends(userId);
    return userModel_.getUserFriends(userID);
}

std::string ChatServer::userFriendsToString(std::vector<std::pair<std::string, std::string>> /*&*/ friends)
{
    //{"code":"1000","sender":"1234","recver":"1235","time":"2022-02-02 14:05:34","body":{"1236":"username","1237":"nameuser"}}
    std::string fs("{");
    for (int i = 0; i < friends.size(); ++i)
    {
        fs = fs + "\"" + friends[i].first + "\":\"" + friends[i].second + "\"";
        if (i != friends.size() - 1)
            fs = fs + ",";
    }
    fs = fs + "}";
    return fs;
}

bool ChatServer::loginValid(const std::string userid, const std::string password)
{
    std::string userMsg = userModel_.validUser(userid, password);
    jsonItem_.parseJson(&*userMsg.begin(), &*userMsg.end());
    if (jsonItem_.jsonItem.count("userid"))
    {
        if (jsonItem_.jsonItem["userid"] != "0")
            return true;
    }
    return false;
}

bool ChatServer::checkUserIsExist(std::string userID)
{
    std::string userMsg = userModel_.getUserInfo(userID);
    jsonItem_.parseJson(&*userMsg.begin(), &*userMsg.end());
    if (jsonItem_.jsonItem.count("userid"))
    {
        if (jsonItem_.jsonItem["userid"] != "0")
            return true;
    }
    return false;
}

std::string ChatServer::getUserInfo(std::string userID)
{
    std::string userMsg = userModel_.getUserInfo(userID);
    return userMsg;
}

FriendState ChatServer::checkIsFriend(std::string user1ID, std::string user2ID)
{
    if (user1ID == systemId)
        return FRIEND;
    return userModel_.checkFriend(user1ID, user2ID);
}

bool ChatServer::agreeRequestOfAddFriend(std::string userID, std::string applyID)
{
    bool existReq = userModel_.checkRequestOfAddFriend(userID, applyID);
    if (!existReq)
    {
        return false;
    }
    else
    {
        userModel_.agreeRequestOfAddFriend(userID, applyID);
        return true;
    }
}

void ChatServer::doLoginAction(const TcpConnectionPtr &conn, std::string userID, std::string password)
{
    //处理用户登录
    bool vaild = loginValid(userID, password);
    if (vaild)
    {
        //验证成功
        //这里要先判断该用户是否已经在线，如果在线的话就把旧的连接踢出去，加入新的连接
        //如果conn已经存在userConnMaps中，说明不可能是登录消息
        if (getUserState(userID)) //用户已经在线
        {
            TcpConnectionPtr preConn;
            {
                MutexLockGuard lock(mutex);
                preConn = userConnMaps_[userID];
                userConnMaps_.erase(userID); //删除旧连接，添加新连接
            }
            //这里要给原客户发一条消息，提示它被挤出去了
            sendMessage(preConn, "1012", systemId, userID, getNowTime(), "被挤占下线", false);
            preConn->shutdown();
            preConn.reset();
        }
        {
            MutexLockGuard lock(mutex);
            userConnMaps_.insert({userID, conn});
        }
        //给客户端发消息说登录成功
        //同时需要找到当用户不在线时，是否有别的用户给这位用户发消息，在这里把这些消息也要发送给他
        sendMessage(conn, "1000", systemId, userID, getNowTime(), "登录成功", false);
        //这里要用redis来做用户的收件箱， 从收件箱中找有没有没收到的信息
        // key=userid, value=[msg1, msg2, ...]
        sendOfflineMessageAction(conn, userID);
        doOnlineNotifyAction(userID);       //通知他的好友他上线了
    }
    else
    {
        //验证失败
        //给客户端发消息说登录失败
        sendMessage(conn, "1011", systemId, userID, getNowTime(), "登录失败(用户名或密码错误)", false);
        conn->shutdown();
    }
}

void ChatServer::doGetFriendsListAction(const TcpConnectionPtr &conn, std::string userID)
{
    //获取好友列表
    std::string friends = userFriendsToString(getUserFriends(userID));
    sendMessage(conn, "1020", systemId, userID, getNowTime(), friends, true);
}

void ChatServer::doAddFriendAction(const TcpConnectionPtr &conn, std::string userID, std::string addedID)
{
    //处理添加好友
    // userID：发起添加的一方
    // addedID：被添加的那一方
    bool exist = checkUserIsExist(addedID);
    if (!exist) //不存在这个人
    {
        sendMessage(conn, "1000", systemId, userID, getNowTime(), "该用户不存在!", false);
    }
    else
    {
        //先要找该用户是否已经有了申请添加别的用户且未被处理的请求
        //防止出现多次请求添加同一人
        FriendState isFriend = checkIsFriend(userID, addedID);
        if (isFriend != FRIEND)
        {
            bool existReq = userModel_.checkRequestOfAddFriend(addedID, userID);
            if (!existReq)
            {
                //不是好友关系，发送好友申请
                sendMessage(conn, "1000", systemId, userID, getNowTime(), "已发送好友申请!", false);
                userModel_.addFriend(userID, addedID);
                if (getUserState(addedID)) //被加的用户在线
                {
                    TcpConnectionPtr recvConn = userConnMaps_[addedID];
                    sendMessage(recvConn, "1005", systemId, addedID, getNowTime(), userID + "申请加您为好友!", false);
                }
                else
                {
                    //该用户不在线，把消息缓存起来
                    cacheMessage("1005", systemId, addedID, getNowTime(), userID + "申请加您为好友!");
                    // std::cout << body << "不在线" << std::endl;
                }
            }
            else
            {
                //已经申请添加他为好友，禁止重复申请
                sendMessage(conn, "1000", systemId, userID, getNowTime(), "您已申请加对方为好友，请等待对方验证！", false);
            }
        }
        else
        {
            //是好友关系
            sendMessage(conn, "1000", systemId, userID, getNowTime(), "该用户已经是您的好友!", false);
        }
    }
}

void ChatServer::doGetUserMessageAction(const TcpConnectionPtr &conn, std::string userID, std::string searchID)
{
    //查看用户信息
    // userID：发起查找的那一方
    // searchID：被查找的那一方
    bool exist = checkUserIsExist(searchID);
    if (!exist) //不存在这个人
    {
        sendMessage(conn, "1000", systemId, userID, getNowTime(), "该用户不存在!", false);
    }
    else
    {
        std::string userMsg = getUserInfo(searchID);
        sendMessage(conn, "1000", systemId, userID, getNowTime(), userMsg, true);
    }
}

void ChatServer::doAgreeReqOfAddFriend(const TcpConnectionPtr &conn, std::string userID, std::string applyID)
{
    //被加的用户处理好友申请
    // code=1007,body=要同意的那个人的userid
    //先在addFriends表中查看是不是有加这个人的请求
    //如果有，将state改为1，代表同意，接着在friends里找有没有他们好友的记录
    //如果有，state改为0，如果没有，新增，state=1
    //发送一条消息给userid，告诉他添加成功
    bool ret = agreeRequestOfAddFriend(userID, applyID);
    if (!ret)
    {
        sendMessage(conn, "1007", systemId, userID, getNowTime(), applyID + "没有申请添加您为好友!", false);
    }
    else
    {
        sendMessage(conn, "1007", systemId, userID, getNowTime(), "您已同意好友请求！" + applyID, false);
        //给apply发一个消息说请求通过
    }
}

void ChatServer::doRefuseReqOfAddFriend(const TcpConnectionPtr &conn, std::string userID, std::string applyID)
{
    //处理好友申请
    // code=1008,body=要同意的那个人的userid
    //现在addFriends表中查看是不是userid有加这个人的请求
    //发送一条消息给userid，告诉他添加失败
    bool existReq = userModel_.checkRequestOfAddFriend(userID, applyID);
    if (!existReq)
        sendMessage(conn, "1007", systemId, userID, getNowTime(), applyID + "没有申请添加您为好友!", false);
    else
    {
        userModel_.refuseRequestOfAddFriend(userID, applyID);
        //给apply发消息说请求被拒绝
    }
}

void ChatServer::doSendMessageAction(const TcpConnectionPtr &conn, std::string code, std::string sendID, std::string recvID, std::string sendTime, std::string message)
{
    //处理发送消息
    // sendID：主动发送消息的那个人
    // recvID：接收消息的那个人
    // message：消息内容
    //一个客户给另一个客户发了一条消息
    //要先判断recver是不是sender的好友。如果是，如果在线，直接发送，如果不在线，缓存消息
    //如果不是好友关系，则返回发送失败
    //系统账号可以给所有人发消息
    FriendState isFriend = checkIsFriend(sendID, recvID);
    if (isFriend == FRIEND) //存在好友关系
    {
        if (getUserState(recvID)) //用户在线
        {
            TcpConnectionPtr recvConn = userConnMaps_[recvID];
            sendMessage(recvConn, code, sendID, recvID, sendTime, message, false);
        }
        else
        {
            //该用户不在线，把消息缓存起来
            cacheMessage(code, sendID, recvID, sendTime, message);
            std::cout << recvID << "不在线" << std::endl;
        }
    }
    else //不存在好友关系
    {
        sendMessage(conn, "1013", systemId, sendID, getNowTime(), recvID + "不是您的好友", false);
    }
}

void ChatServer::doGetMyFriendReqAction(const TcpConnectionPtr &conn, std::string userID)
{
    //查看我的好友申请：分为我申请加的和申请加我的
    std::vector<std::pair<std::string, std::string>> listOfIAdd = userModel_.getAddReqList(userID);       //我主动发起添加的
    std::vector<std::pair<std::string, std::string>> listOfIAdded = userModel_.getAddedReqList(userID);     //添加我的请求
    sendMessage(conn, "1000", systemId, userID, getNowTime(), userFriendsToString(listOfIAdd), true);
    sendMessage(conn, "1000", systemId, userID, getNowTime(), userFriendsToString(listOfIAdded), true);
}

void ChatServer::doOfflineNotifyAction(std::string userID)
{
    //当用户下线时，通知他的所有在线好友他下线了
    std::vector<std::pair<std::string, std::string>> friends = getUserFriends(userID);
    for (auto &f : friends)
    {
        bool state = getUserState(f.first); //获取朋友的在线状态
        if (state)
        {
            //如果在线的话向该朋友发送通知
            TcpConnectionPtr fConn = userConnMaps_[f.first]; // fix:这里会有问题，需要修复，要加锁
            sendMessage(fConn, "1000", systemId, f.first, getNowTime(), "您的朋友已下线！:" + userID, false);
        }
    }
}

void ChatServer::doOnlineNotifyAction(std::string userID)
{
    //上线提醒,通知他的好友他上线了
    std::vector<std::pair<std::string, std::string>> friends = getUserFriends(userID);
    for (auto &f : friends)
    {
        bool state = getUserState(f.first); //获取朋友的在线状态
        if (state)
        {
            //如果在线的话向该朋友发送通知
            TcpConnectionPtr fConn = userConnMaps_[f.first]; // fix:这里会有问题，需要修复，要加锁
            sendMessage(fConn, "1000", systemId, f.first, getNowTime(), "您的朋友已上线！:" + userID, false);
        }
    }
}

void ChatServer::doDeleteFriendAction(const TcpConnectionPtr &conn, std::string userID, std::string deleteID)
{
    //删除好友
    //首先查看是否存在好友关系
    FriendState state = checkIsFriend(userID, deleteID);
    if(state==FRIEND)
    {
        userModel_.deleteFriend(userID, deleteID);
        sendMessage(conn, "1000", systemId, userID, getNowTime(), "删除成功!");
    }
    else
    {
        sendMessage(conn, "1000", systemId, userID, getNowTime(), deleteID + "不是您的好友！");
    }
}

void ChatServer::sendOfflineMessageAction(const TcpConnectionPtr &conn, std::string userID)
{
    //发送用户的离线消息
    std::map<std::string, std::vector<std::string>>::iterator it;
    int count = 0;
    {
        MutexLockGuard lock(mutex);
        it = cachedMessages_.find(userID);
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

std::string ChatServer::getNowTime()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    std::string year = std::to_string(1900 + ltm->tm_year) + "/";
    std::string month = std::to_string(1 + ltm->tm_mon) + "/";
    std::string day = std::to_string(ltm->tm_mday) + " ";
    std::string hour = std::to_string(ltm->tm_hour) + ":";
    std::string mins = std::to_string(ltm->tm_min) + ":";
    std::string sec = std::to_string(ltm->tm_sec);
    return year + month + day + hour + mins + sec;
}