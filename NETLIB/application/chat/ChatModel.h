//
#include <string>
#include <vector>

enum FriendState
{
    NOTFRIEND,  //非好友关系
    WEAKFRIEND, //一方向另一方申请加好友
    FRIEND      //好友关系
};
//好友关系逻辑：friends表和addFriends表
// friends表里面没有记录：不是互相的好友
// friends表里state=0:曾经加过好友但是后来删了
// friends表里state=1:互为好友关系
// addFriends表记录添加好友的信息
// applyuser:申请的那个人
// addeduser:被添加的那个人
// state:0:拒绝， 1:同意

class UserModel
{
    // private:
    std::string userID_;
    std::string username_;
    std::string password_;
    // std::vector<std::string> messages_;
public:
    std::string getUserID()
    {
        return userID_;
    }

    std::string getUsername()
    {
        return username_;
    }
    //获取user信息
    //更新用户信息
    std::string getUserInfo(std::string userID);
    std::string validUser(std::string userID, std::string password);
    std::vector<std::pair<std::string, std::string>> getUserFriends(std::string userID);
    std::vector<std::pair<std::string, std::string>> getAddReqList(std::string userID);
    std::vector<std::pair<std::string, std::string>> getAddedReqList(std::string userID);
    FriendState checkFriend(std::string user1ID, std::string user2ID);
    void addFriend(std::string user1ID, std::string user2ID);
    void deleteFriend(std::string userID, std::string deleteID);
    bool checkRequestOfAddFriend(std::string userID, std::string applyID);
    void agreeRequestOfAddFriend(std::string userID, std::string applyID);
    void refuseRequestOfAddFriend(std::string userID, std::string applyID);
};