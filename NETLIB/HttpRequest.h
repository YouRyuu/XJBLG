#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#include <string>
#include <map>
#include <assert.h>
class HttpRequest
{
public:
    enum Method
    {
        Invalid,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE
    };

    enum Version
    {
        Unknown,
        Http10,
        Http11
    };

    HttpRequest()
        : method_(Invalid), version_(Unknown)
    {
    }

    void setVersion(Version v)
    {
        version_ = v;
    }

    void setMethod(Method m)
    {
        method_ = m;
    }

    Version getVersion() const
    {
        return version_;
    }

    Method getMethod() const
    {
        return method_;
    }

    bool setMethod(const char *start, const char *end) // Buffer中存的是vector<char>
    {
        assert(method_ == Invalid);
        std::string m(start, end);
        if (m == "GET")
        {
            method_ = GET;
        }
        else if (m == "POST")
        {
            method_ = POST;
        }
        else if (m == "HEAD")
        {
            method_ = HEAD;
        }
        else if (m == "PUT")
        {
            method_ = PUT;
        }
        else if (m == "DELETE")
        {
            method_ = DELETE;
        }
        else
        {
            method_ = Invalid;
        }
        return method_ != Invalid;
    }

    const char *methodString() const
    {
        const char *result = "UNKNOWN";
        switch (method_)
        {
        case GET:
            result = "GET";
            break;
        case POST:
            result = "POST";
            break;
        case HEAD:
            result = "HEAD";
            break;
        case PUT:
            result = "PUT";
            break;
        case DELETE:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char *start, const char *end)
    {
        path_.assign(start, end);
    }

    const std::string &getPath() const
    {
        return path_;
    }

    void setQuery(const char *start, const char *end)
    {
        query_.assign(start, end);
    }

    const std::string &getQuery() const
    {
        return query_;
    }

    void setBody(const char *start, const char *end)
    {
        body_.assign(start, end);
    }

    const std::string getBody() const
    {
        return body_;
    }

    void addHeader(const char *start, const char *colon, const char *end)
    {
        //使用方法：request_.addHeader(buf->peek(), colon, crlf);     colon是":"
        //报文格式：
        // field1: value1\r\n
        // field2: value2\r\n
        //\r\n
        // body
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon))
        {
            //跳过空格
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1])) //找到最后一个不是空格的地方\r\n
        {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string &field) const
    {
        std::string value;
        auto it = headers_.find(field);
        if (it != headers_.end())
        {
            value = it->second;
        }
        return value;
    }

    const std::map<std::string, std::string> getHeaders() const
    {
        return headers_;
    }

    void swap(HttpRequest &that)
    {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        headers_.swap(that.headers_);
    }

private:
    Method method_;     //请求方法
    Version version_;       //HTTP版本
    std::string path_;      //请求路径
    std::string query_;     //查询字符串
    std::map<std::string, std::string> headers_;        //请求头部
    std::string body_;      //post请求体
};

#endif