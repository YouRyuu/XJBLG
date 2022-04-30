#include "../../mysql/mysqlDB.h"
#include "ChatModel.h"

std::string UserModel::getUserInfo(std::string userID)
{
    //{"code":0}
    //{"code":1, "userid":"1111", "username":"bm"}
    std::string ret;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string sql("select userid, username from users where userid=\"");
    sql = sql + userID;
    sql = sql + "\"";
    res = db.execute(sql);
    if (res->rowsCount() == 0)
    {
        ret = "{\"userid\":\"0\"}";
    }
    else
    {
        assert(res->rowsCount() == 1);
        //序列化结果
        //{"userid":"1111", "username":"bm"}
        while (res->next())
        {
            userID_ = res->getString("userid");
            username_ = res->getString("username");
            ret = "{\"userid\":\"";
            ret = ret + userID_;
            ret = ret + "\",\"username\":\"";
            ret = ret + username_;
            ret = ret + "\"}";
        }
    }
    return ret;
}

std::string UserModel::validUser(std::string userID, std::string password)
{
    std::string ret;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string sql("select userid, username from users where userid=\"");
    sql = sql + userID;
    sql = sql + "\" and password=\"";
    sql = sql + password + "\"";
    res = db.execute(sql);
    if (res->rowsCount() == 0)
    {
        ret = "{\"userid\":\"0\"}";
    }
    else
    {
        assert(res->rowsCount() == 1);
        //序列化结果
        //{"userid":"1111", "username":"bm"}
        while (res->next())
        {
            userID_ = res->getString("userid");
            username_ = res->getString("username");
            ret = "{\"userid\":\"";
            ret = ret + userID_;
            ret = ret + "\",\"username\":\"";
            ret = ret + username_;
            ret = ret + "\"}";
        }
    }
    return ret;
}

std::vector<std::pair<std::string, std::string>> UserModel::getUserFriends(std::string userID)
{
    //获取好友列表
    // select userid, username from users where userid in (select user1 from friends where user2=11111111 union select user2 from friends where user1=11111111);
    std::string ret;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string sql("select userid, username from users where userid in (select user1 from friends where user2=");
    sql = sql + userID;
    sql = sql + " and state=1 union select user2 from friends where user1=";
    sql = sql + userID;
    sql = sql + " and state=1)";
    res = db.execute(sql);
    std::vector<std::pair<std::string, std::string>> friends;
    if (res->rowsCount() == 0)
        return friends;
    else
    {
        while (res->next())
        {
            std::string user = res->getString("userid");
            std::string name = res->getString("username");
            friends.push_back({user, name});
        }
    }
    return friends;
}

FriendState UserModel::checkFriend(std::string user1ID, std::string user2ID)
{
    //检查user1和user2是否为好友关系
    // select state from friends where user1=11111111 and user2=100000000 union select state from friends where user2=11111111 and user1=10000000;
    // state:0,1  如果没有查询到state，说明user1不是user2的好友，可以发送申请，如果state是0，说明user1已经向user2发送过好友申请
    std::string sql("select state from friends where user1=");
    sql = sql + user1ID;
    sql = sql + " and user2=";
    sql = sql + user2ID;
    sql = sql + " union select state from friends where user2=";
    sql = sql + user1ID;
    sql = sql + " and user1=";
    sql = sql + user2ID;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    res = db.execute(sql);
    if (res->rowsCount() == 0)
        return NOTFRIEND;
    else
    {
        while (res->next())
        {
            std::string state = res->getString("state");
            if (state == "1")
                return FRIEND;
            else
                return WEAKFRIEND;
        }
    }
}

void UserModel::addFriend(std::string user1ID, std::string user2ID)
{
    // insert into addFriends(user1, user2, state) values(user1ID, user2ID, 0);
    //加好友申请记录表
    std::string sql("insert into addFriends(applyUser,addedUser,state) values(");
    sql = sql + user1ID;
    sql = sql + ",";
    sql = sql + user2ID;
    sql = sql + ",0)";
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    res = db.execute(sql);
}

void UserModel::deleteFriend(std::string userID, std::string deleteID)
{
    //删除好友
    // update friends set state=0 where (user1=userID and user2=deleteID) or (user1=deleteID and user2=userID)
    std::string sql("update friends set state=0 where (user1=");
    sql = sql + userID;
    sql = sql + " and user2=";
    sql = sql + deleteID;
    sql = sql + ") or (user1=";
    sql = sql + deleteID;
    sql = sql + " and user2=";
    sql = sql + userID;
    sql = sql + ")";
    MysqlDB db;
    db.execute(sql);
}

bool UserModel::checkRequestOfAddFriend(std::string userID, std::string applyID)
{
    //查询userID是否有被applyID加好友的请求
    std::string sql("select state from addFriends where applyUser=");
    sql = sql + applyID;
    sql = sql + " and addedUser=";
    sql = sql + userID;
    sql = sql + " and state=0";
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    res = db.execute(sql);
    if (res->rowsCount() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void UserModel::refuseRequestOfAddFriend(std::string userID, std::string applyID)
{
    std::string sql("update addFriends set state=2 where applyUser=");
    sql = sql + applyID;
    sql = sql + (" and addedUser=");
    sql = sql + userID;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    res = db.execute(sql);
}

void UserModel::agreeRequestOfAddFriend(std::string userID, std::string applyID)
{
    // userID同意applyID请求添加userID为好友的申请
    std::string sql("update addFriends set state=1 where applyUser=");
    sql = sql + applyID;
    sql = sql + (" and addedUser=");
    sql = sql + userID;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    res = db.execute(sql);
    FriendState state = checkFriend(userID, applyID);
    if (state == WEAKFRIEND)
    {
        sql = "update friends set state=1 where (user1=";
        sql = sql + userID;
        sql = sql + " and user2=";
        sql = sql + applyID;
        sql = sql + ") or (user1=";
        sql = sql + applyID;
        sql = sql + " and user2=";
        sql = sql + userID;
        sql = sql + ")";
        res = db.execute(sql);
    }
    else if (state == NOTFRIEND)
    {
        sql = "insert into friends(user1,user2,state) values(";
        sql = sql + userID;
        sql = sql + ",";
        sql = sql + applyID;
        sql = sql + ",";
        sql = sql + "1)";
        res = db.execute(sql);
    }
}

std::vector<std::pair<std::string, std::string>> UserModel::getAddReqList(std::string userID)
{
    //获取我主动发起添加好友的请求列表
    //select addedUser,state from addFriends where applyUser=userID;
    //返回：pair<string, string>  --> pair<我申请添加好友的id，状态>
    std::string sql("select addedUser,state from addFriends where applyUser=");
    sql = sql + userID;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::vector<std::pair<std::string, std::string>> lists;
    res = db.execute(sql);
    while(res->next())
    {
        std::string id = res->getString("addedUser");
        std::string state = res->getString("state");
        lists.push_back({id, state});
    }
    return lists;
}

std::vector<std::pair<std::string, std::string>> UserModel::getAddedReqList(std::string userID)
{
    //获取我被添加好友的请求列表
    //返回：pair<string, string>  --> pair<申请添加我为好友的一方id，状态>
    std::string sql("select applyUser,state from addFriends where addedUser=");
    sql = sql + userID;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::vector<std::pair<std::string, std::string>> lists;
    res = db.execute(sql);
    while(res->next())
    {
        std::string id = res->getString("applyUser");
        std::string state = res->getString("state");
        lists.push_back({id, state});
    }
    return lists;
}
