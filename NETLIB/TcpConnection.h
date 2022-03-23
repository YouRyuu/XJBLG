#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include <memory>
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "StringPiece.h"
#include "HttpContext.h"
class Channel;
class EventLoop;
class Socket;
class HttpContext;

class TcpConnection: public std::enable_shared_from_this<TcpConnection>
{
    public:
        TcpConnection(EventLoop* loop, const std::string&name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
        ~TcpConnection();
        const std::string& name() { return name_;};
        const InetAddress& localAddr() const {
            return localAddr_;
        }
        const InetAddress& peerAddr() const {
            return peerAddr_;
        }
        bool connected() const { return state_ == Connected; }
        bool disconnected() const { return state_ == Disconnected; }
        EventLoop* getLoop() const { return loop_; }
        void send(const StringPiece& data);    //发送数据
        void send(const void* data, int len);
        void send(Buffer *message);
        void shutdown();
        void forceClose();
        void startRead();
        void startReadInLoop();
        void stopRead();
        void stopReadInLoop();
        bool isReading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop

          void setContext(const HttpContext& context)
          { context_ = context; }

          const HttpContext& getContext() const
          { return context_; }

          HttpContext* getMutableContext()
          { return &context_; }

        void setConnectionCallback(const ConnectionCallback& cb)
        { connectionCallback_ = cb; }

        void setMessageCallback(const MessageCallback& cb)
        { messageCallback_ = cb; }

        void setWriteCompleteCallback(const WriteCompleteCallback& cb)
        { writeCompleteCallback_ = cb; }

        //   void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
        //   { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

        void setCloseCallback(const CloseCallback& cb)
        { closeCallback_ = cb; }

        void connectEstablished();   // should be called only once
        void connectDestroyed();  // should be called only once

        void setTcpNoDelay(bool on);

        Buffer* inputBuffer()
        {
            return &inputBuffer_;
        }

        Buffer* outputBuffer()
        {
            return &outputBuffer_;
        }

    private:
        enum StateE { Disconnected, Connecting, Connected, Disconnecting};
        void handleRead();
        void handleWrite();
        void handleClose();
        void handleError();
        void setState(StateE s)
        {
            state_ = s;
        }

        void sendInLoop(const StringPiece& message);
        void sendInLoop(const void* message, size_t len);
        void shutdownInLoop();
        void forceCloseInLoop();
        const char* stateToString() const;

        EventLoop* loop_;
        const std::string name_;
        StateE state_;
        bool reading_;
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;
        const InetAddress localAddr_;
        const InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        CloseCallback closeCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        MessageCallback messageCallback_;
        Buffer inputBuffer_;
        Buffer outputBuffer_;
        HttpContext context_;
};

#endif