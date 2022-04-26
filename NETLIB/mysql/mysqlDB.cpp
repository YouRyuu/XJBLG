#include "mysqlDB.h"
#include <iostream>
int main()
{
    MysqlDB db;
    MysqlDB::ResultSetPtr res;
    std::string userid("11111112");
    std::string password("bmniubi");
    std::string sql("select userid from users where userid=\"");
    sql = sql + userid;
    sql = sql + "\" and password=\"";
    sql = sql + password + "\"";
    std::cout<<sql<<std::endl;
    res = db.execute(sql);
    printf("row:%ld\n", res->rowsCount());
    while(res->next())
    {
        printf("userid:%s\n", res->getString("userid").c_str());
    }
}