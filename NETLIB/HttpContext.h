#ifndef HTTPCONTENT_H
#define HTTPCONTENT_H
#include "HttpRequest.h"
class Buffer;

class HttpContext
{
public:
    enum HttpRequestParseState //解析状态
    {
        RequestLine, //请求头
        Headers,     //请求头部
        Body,        //报文体
        GotAll       //完成所有解析
    };

    HttpContext():state_(RequestLine)
    {
    }

    bool parseRequest(Buffer *buf);

    bool gotAll()
    {
        return state_ == GotAll;
    }

    void reset()
    {
        state_ = RequestLine;
        HttpRequest req;
        req_.swap(req);
    }

    const HttpRequest &request() const
    {
        return req_;
    }

    HttpRequest &request()
    {
        return req_;
    }

private:
    bool processRequestLine(const char *begin, const char *end);
    HttpRequestParseState state_;
    HttpRequest req_;
};

#endif