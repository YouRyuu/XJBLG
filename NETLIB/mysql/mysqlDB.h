#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>
#include <memory>

#define HOST "tcp://127.0.0.1:3306/IM"
#define USER "root"
#define PASSWORD "o"

class MysqlDB
{
    public:
    typedef std::shared_ptr<sql::ResultSet> ResultSetPtr;
    MysqlDB():d(get_driver_instance()), resultSetPtr()
    {
        conn = d->connect(HOST, USER, PASSWORD);
        stmt = conn->createStatement();
    }
    ~MysqlDB()
    {
        if(stmt)
            delete stmt;
        if(conn)
            delete conn;
    }

    ResultSetPtr execute(std::string sqlMent)
    {
        //std::cout<<sqlMent<<std::endl;
        if(stmt->execute(sqlMent))      //select操作execute返回true,反之返回false
        {
            resultSetPtr.reset(stmt->getResultSet());
            return resultSetPtr;
        }
        return nullptr;
    }

    private:
        sql::Driver *d;
        sql::Connection *conn;
        sql::Statement *stmt;
        ResultSetPtr resultSetPtr;
};