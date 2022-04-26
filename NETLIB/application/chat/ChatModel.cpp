#include "../../mysql/mysqlDB.h"
#include "ChatModel.h"

std::string ChatModel::getUserInfo(std::string userId)
{
    std::string ret;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string sql("select userid, username from users where userid=\"");
    sql = sql + userId;
    sql = sql + "\"";
    res = db.execute(sql);
    if (res->rowsCount() == 0)
        return ret;
    else
    {
        //序列化结果
        //{"userid":"1111", "username":"bm"}
        while (res->next())
        {
            userId_ = res->getString("userid");
            username_ = res->getString("username");
            ret = userId_ + username_;
        }
        return ret;
    }
}

std::string ChatModel::validUser(std::string userId, std::string password)
{
    std::string ret;
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string sql("select userid, username from users where userid=\"");
    sql = sql + userId;
    sql = sql + "\" and password=\"";
    sql = sql + password + "\"";
    res = db.execute(sql);
    if (res->rowsCount() == 0)
        return ret;
    else
    {
        //序列化结果
        //{"userid":"1111", "username":"bm"}
        while (res->next())
        {
            userId_ = res->getString("userid");
            username_ = res->getString("username");
            ret = userId_ + username_;
        }
        return ret;
    }
}
