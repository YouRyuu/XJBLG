//
#include <string>
//#include <vector>
class ChatModel
{
    //private:
        std::string userId_;
        std::string username_;
        //std::vector<std::string> messages_;
    public:
        //获取user信息
        //更新用户信息
        std::string getUserInfo(std::string userId);
        std::string validUser(std::string userId, std::string password);
};