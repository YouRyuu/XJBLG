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
    loop->runEvery(60, std::bind(&ChatServer::sendHeartPacket, this));
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
            for (std::map<std::string, WeakTcpConnectionPtr>::iterator it = userConnMaps_.begin(); it != userConnMaps_.end(); ++it)
            {
                if ((it->second).lock() == conn)
                {
                    userID = it->first;
                    userConnMaps_.erase(it);
                    break;
                }
            }
        }
        if (userID.size() > 0) //因为如果用户是被挤下去的话，在登录处理里面这个连接就会被删掉，此时获取不到userID。
            doOfflineNotifyAction(userID);
    }
}

void ChatServer::onContextMessage(const TcpConnectionPtr &conn, ChatContext &chatContext, int n)
{
    onRequest(conn, chatContext);
}

void ChatServer::onRequest(const TcpConnectionPtr &conn, ChatContext &chatContext)
{
    int32_t length = chatContext_.getLength();
    std::string code = chatContext_.getCode();
    std::string seq = chatContext_.getSeq();
    std::string sender = chatContext_.getSender();
    std::string recver = chatContext_.getRevcer();
    std::string time = chatContext_.getTime();
    std::string body = chatContext_.getBody();
    // todo：这里要改成策略模式
    if (code == "1001")
    {
        doLoginAction(conn, sender, body);
    }
    else if (code == "1111")
    {
        //确认ack消息
        doAckAction(body);
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
    else if (code == "1009")
    {
        //查看好友申请
        doGetMyFriendReqAction(conn, sender);
    }
    else if (code == "1010")
    {
        //删除好友
        doDeleteFriendAction(conn, sender, body);
    }
    else if(code=="1011")
    {
        //心跳包
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

std::pair<std::string, std::string> ChatServer::encodeMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList)
{
    //---------------------------//
    //{"code":"1111","seq":"45636","sender":"1122","recver":"3344","time":"222222222","body":"xxxxx"}
    //{"code":"1","userid":"11111111","username":"bm"}
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
    insertMessageToWindow(seq, recver, msg);
    std::cout << "发送序列号为:" << seq << "的消息" << std::endl;
    return {seq, msg};
}

void ChatServer::sendMessage(const TcpConnectionPtr &conn, std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList)
{
    //消息处理逻辑：
    //用户不在线：直接存到离线消息中
    //用户在线：
    //      将消息存放到消息窗口,并注册消息超时函数
    //          如果消息得到确认：
    //              将消息从窗口中移除
    //          如果消息没有得到确认（超时，这时可认为客户离线了）：
    //              将消息存放到离线消息中
    //              在窗口中移除此消息
    //              清除该离线的用户
    std::pair<std::string, std::string> context = encodeMessage(code, sender, recver, time, body, bodyIsList);
    if (!conn) // recver用户不在线
    {
        cacheMessage(recver, context.second);
    }
    else
    {
        //注册超时函数
        //
        if(code!="1011")        //不是心跳包
            conn->getLoop()->runAfter(30, std::bind(&ChatServer::handleNotAckMessage, this, context.first, false));
        else
            conn->getLoop()->runAfter(30, std::bind(&ChatServer::handleNotAckMessage, this, context.first, true));
        sendMessage(conn, context.second);
    }
}

void ChatServer::sendMessage(std::string code, std::string sender, std::string recver, std::string time, std::string body, bool bodyIsList)
{
    if (getUserState(recver)) //用户在线
    {
        TcpConnectionPtr recvConn = userConnMaps_.find(recver)->second.lock();
        sendMessage(recvConn, code, sender, recver, time, body, bodyIsList);
    }
    else
    {
        //该用户不在线，把消息缓存起来
        sendMessage(nullptr, code, sender, recver, time, body, bodyIsList);
    }
}

void ChatServer::cacheMessage(std::string recver, std::string message)
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
void ChatServer::insertMessageToWindow(std::string seq, std::string recver, std::string message)
{
    assert(messageWindow_.count(seq) == 0);
    messageWindow_[seq] = {recver, message};
}

void ChatServer::deleteMessageFromWindow(std::string seq)
{
    assert(messageWindow_.count(seq) == 1);
    messageWindow_.erase(seq);
    std::cout << "清除序列号为" << seq << "的消息" << std::endl;
}

bool ChatServer::getUserState(std::string userId)
{
    //参数：要查找的用户的id
    //根据用户id获取用户的在线状态
    //若在线返回true，反之返回false
    bool state = false;
    {
        MutexLockGuard lock(mutex);
        std::map<std::string, WeakTcpConnectionPtr>::iterator it = userConnMaps_.find(userId);
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
            UserConnMapsIt it;
            {
                MutexLockGuard lock(mutex);
                it = userConnMaps_.find(userID);
                if (it != userConnMaps_.end())
                {
                    preConn = it->second.lock();
                }
                userConnMaps_.erase(userID); //删除旧连接，添加新连接
            }
            //这里要给原客户发一条消息，提示它被挤出去了
            if (preConn)
            {
                sendMessage(preConn, "1012", systemId, userID, getNowTime(), "被挤占下线", false);
                preConn->shutdown();
                preConn.reset();
            }
        }
        WeakTcpConnectionPtr wConn(conn);
        {
            MutexLockGuard lock(mutex);
            userConnMaps_.insert({userID, wConn});
        }
        //给客户端发消息说登录成功
        //同时需要找到当用户不在线时，是否有别的用户给这位用户发消息，在这里把这些消息也要发送给他
        sendMessage(conn, "1000", systemId, userID, getNowTime(), "登录成功", false);
        //这里要用redis来做用户的收件箱， 从收件箱中找有没有没收到的信息
        // key=userid, value=[msg1, msg2, ...]
        sendOfflineMessageAction(conn, userID);
        doOnlineNotifyAction(userID); //通知他的好友他上线了
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
                sendMessage("1005", systemId, addedID, getNowTime(), userID + "申请加您为好友!");
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
        sendMessage("1007", systemId, applyID, getNowTime(), userID + "已同意您的好友请求!");
    }
}

void ChatServer::doRefuseReqOfAddFriend(const TcpConnectionPtr &conn, std::string userID, std::string applyID)
{
    //处理好友申请
    // code=1008,body=要同意的那个人的userid
    //现在addFriends表中查看是不是userid有加这个人的请求.如果有，将state改为拒绝
    //发送一条消息给userid，告诉他添加失败
    bool existReq = userModel_.checkRequestOfAddFriend(userID, applyID);
    if (!existReq)
        sendMessage(conn, "1007", systemId, userID, getNowTime(), applyID + "没有申请添加您为好友!", false);
    else
    {
        userModel_.refuseRequestOfAddFriend(userID, applyID);
        //给apply发消息说请求被拒绝
        sendMessage("1008", systemId, applyID, getNowTime(), userID + "拒绝了您的好友请求!");
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
    //在这里有个问题：服务端转发完消息后要确认客户端是否收到
    //即服务端在转发完消息之后要等待客户端的回执
    //如果在规定的时间内服务端没有等到回执，就认为对方连接意外关闭了，要把消息存在离线消息里面，并且踢掉这个超时的连接
    FriendState isFriend = checkIsFriend(sendID, recvID);
    if (isFriend == FRIEND) //存在好友关系
    {
        sendMessage(code, sendID, recvID, sendTime, message);
    }
    else //不存在好友关系
    {
        sendMessage(conn, "1013", systemId, sendID, getNowTime(), recvID + "不是您的好友", false);
    }
}

void ChatServer::doGetMyFriendReqAction(const TcpConnectionPtr &conn, std::string userID)
{
    //查看我的好友申请：分为我申请加的和申请加我的
    std::vector<std::pair<std::string, std::string>> listOfIAdd = userModel_.getAddReqList(userID);     //我主动发起添加的
    std::vector<std::pair<std::string, std::string>> listOfIAdded = userModel_.getAddedReqList(userID); //添加我的请求
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
            TcpConnectionPtr fConn = userConnMaps_[f.first].lock(); // fix:这里会有问题，需要修复，要加锁
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
            TcpConnectionPtr fConn = userConnMaps_[f.first].lock(); // fix:这里会有问题，需要修复，要加锁
            sendMessage(fConn, "1000", systemId, f.first, getNowTime(), "您的朋友已上线！:" + userID, false);
        }
    }
}

void ChatServer::doDeleteFriendAction(const TcpConnectionPtr &conn, std::string userID, std::string deleteID)
{
    //删除好友
    //首先查看是否存在好友关系
    FriendState state = checkIsFriend(userID, deleteID);
    if (state == FRIEND)
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

void ChatServer::doAckAction(std::string seq)
{
    //确认ack，删除消息窗口中的函数，并删除注册的超时函数
    assert(messageWindow_.count(seq) == 1);
    messageWindow_.erase(seq);
    std::cout << "删除ack:" << seq << std::endl;
    // debug用
    // printNowMessageWindow();
}

void ChatServer::doOvertimeAction(std::string seq)
{
    //处理超时的窗口消息
    assert(messageWindow_.count(seq) != 0);
    auto it = messageWindow_.find(seq);
    std::string recver = (it->second).first;
    std::string message = (it->second).second;
    cacheMessage(recver, message);
}

void ChatServer::printNowMessageWindow()
{
    if (messageWindow_.size() == 0)
        printf("MessageWindow now is NULL\n");
    else
    {
        for (auto it = messageWindow_.begin(); it != messageWindow_.end(); ++it)
        {
            printf("seq:[%s], recver:[%s], message:[%s]\n", it->first.c_str(), it->second.first.c_str(), it->second.second.c_str());
        }
    }
}

void ChatServer::handleNotAckMessage(std::string seq, bool heart)
{
    //处理消息窗口中超时的消息
    //利用定时器处理，在将消息插入到消息窗口后，激活一个定时器，函数的参数是那个要被监视的seq
    //如果定时器到期，检查该消息是否还在消息窗口中，如果还在，就把它加入到离线消息中，然后调用踢掉超时连接操作
    //如果是心跳包，则不缓存消息
    if(messageWindow_.count(seq)==0)
    {
        std::cout<<"seq already pop from mw"<<std::endl;
        return;
    }
    else
    {
        auto mw = messageWindow_.find(seq);
        assert(mw!=messageWindow_.end());
        if(!heart)      //不是心跳包，存入缓存
            cacheMessage(mw->second.first, mw->second.second);
        closeOvertimeConn(mw->second.first);
        messageWindow_.erase(seq);
        std::cout<<seq<<"的消息已超时"<<std::endl;
    }
}

void ChatServer::closeOvertimeConn(std::string userID)
{
    //强制踢出超时连接
    auto conn = userConnMaps_.find(userID);
    if(conn==userConnMaps_.end())
    {
        std::cout<<"userID already exit from userConnMaps"<<std::endl;
        return;
    }
    else
    {
        TcpConnectionPtr connptr = conn->second.lock();
        userConnMaps_.erase(conn);
        connptr->shutdown();
        std::cout<<userID<<"断开连接，已被踢出"<<std::endl;
    }
}

void ChatServer::sendHeartPacket()     //发送心跳包
{
    UserConnMapsIt userIt;
    for(userIt=userConnMaps_.begin(); userIt!=userConnMaps_.end(); ++userIt)
    {
        TcpConnectionPtr conn = userIt->second.lock();
        sendMessage(conn, "1011", systemId, userIt->first, getNowTime(), "heart");
    }
}

