#include "HttpContext.h"
#include "Buffer.h"
#include <algorithm>
#include <iostream>

bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    //ex: GET /search?hl=zh-CN&source=hp&q=domety&aq=f&oq= HTTP/1.1\r\n
    bool ret = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if(space!=end && req_.setMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if(space!=end)
        {
            const char* problem = std::find(start, space, '?');
            if(problem!=space)
            {
                req_.setPath(start, problem);
                req_.setQuery(problem, space);
            }
            else
            {
                req_.setPath(start, problem);
            }
            start = space + 1;
            ret = end - start == 8 && std::equal(start, end-1, "HTTP/1.");
            if(ret)
            {
                if(*(end-1)=='1')
                {
                    req_.setVersion(HttpRequest::Http11);
                }
                else if(*(end-1)=='0')
                {
                    req_.setVersion(HttpRequest::Http10);
                }
                else
                {
                    ret = false;
                }
            }
        }
    }
    return ret;
}

bool HttpContext::parseRequest(Buffer* buf)
{
    //如果是POST请求，还会有body报文，而GET请求没有
    bool ok = true;
    bool hasMore = true;
    while(hasMore)
    {
        if(state_==RequestLine)     //处理请求头
        {
            const char* crlf = buf->findCRLF();
            if(crlf)
            {
                ok = processRequestLine(buf->peek(), crlf);
                if(ok)      //是规范的请求头部
                {
                    buf->retrieveUntil(crlf+2); //将buf中的请求头读完
                    state_ = Headers;       //开始处理请求头部
                }
                else
                {
                    hasMore = false;    //错误数据，直接返回错误
                }
            }
            else
            {
                hasMore = false;
            }
        }
        else if(state_ == Body)
        {
            //读取POST的body数据
            req_.setBody(buf->peek(), buf->beginWrite());
            buf->retrieveAll();
            state_ = GotAll;
            hasMore = false;
        }
        else if(state_ == Headers)      //处理头部数据
        {
            const char* crlf = buf->findCRLF();     //找第一个头部的结尾\r\n
            if(crlf)
            {
                const char* colon = std::find(buf->peek(), crlf, ':');      //找到分隔符
                if(colon!=crlf)     //存在键值对
                {
                    req_.addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    //不存在键值对了,header已经读完
                    //这时候如果req_.method是get的话，就可以结束了，而如果是post的话， 还需要继续读
                    if(req_.getMethod()==HttpRequest::GET)
                    {
                        state_ = GotAll;
                        hasMore = false;
                    }
                    else if(req_.getMethod()==HttpRequest::POST)
                    {
                        state_ = Body;
                        hasMore = true;
                    }
                }
                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                hasMore = false;
            }
        }
        else
        {
            hasMore = false;
        }
    }
    return ok;
}