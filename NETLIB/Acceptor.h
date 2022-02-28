#ifndef WS_ACCEPTOR_H
#define WS_ACCEPTOR_H

#include "Channel.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

class Acceptor
{
    public:
        typedef std::function<void (int sockfd, InetAddress&)> NewConnectionCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback& cb)
        {
            newCb = cb;
        }

        bool listening() const {
            return listening_;
        }

        void listen();

        private:
            NewConnectionCallback newCb;
            EventLoop* loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            bool listening_;
            int idleFd_;
            void handleRead();
};

#endif