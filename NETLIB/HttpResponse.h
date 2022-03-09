#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

class Buffer;
class HttpResponse
{
    public:
    enum HttpStatusCode
    {
        kunknown,
        k200ok = 200,
        k301 = 301,
        k400 = 400,
        k404 = 404
    };

    HttpResponse(bool close):
    statusCode_(kunknown), closeConnection_(close)
    {}

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_ = code;
    }

    void setStatusMessage( const std::string& message)
    {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on)
    {
        closeConnection_ = on;
    }

    bool closeConnection() const
    {
        return closeConnection_;
    }

    void addHeader(const std::string& key, const std::string& value)
    {
        headers_[key] = value;
    }

    void setContentType(const std::string& type)
    {
        addHeader("Content-Type", type);
    }

    void setBody(const std::string body)
    {
        body_ = body;
    }

    void appendToBuffer(Buffer* output);

    private:
        std::map<std::string ,std::string> headers_;
        HttpStatusCode statusCode_;
        std::string statusMessage_;
        bool closeConnection_;
        std::string body_;
};

#endif