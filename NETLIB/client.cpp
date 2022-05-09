#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/Eventloop.h"
#include "net/Acceptor.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include "net/Buffer.h"
#include "net/StringPiece.h"
#include "net/ThreadPool.h"
#include "net/CurrentThread.h"
#include "net/Connector.h"
#include "net/TcpClient.h"
#include "net/EventLoopThread.h"
#include "application/chat/ChatClient.h"
#include "application/http/HttpResponse.h"
#include "application/http/HttpServer.h"
#include "application/http/HttpRequest.h"

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        EventLoop loop;
        EventLoopThread loopThread;
        InetAddress addr(std::string("101.35.102.189"), 6666);
        ChatClient client(loopThread.startLoop(), addr, "ChatClient");
        //ChatClient client(&loop, addr, "ChatClient");
        client.setUserId(argv[1]);
        client.setUserPassword(argv[2]);
        client.connect();
        //loop.loop();
        std::string line;
        while(std::getline(std::cin, line))
        {
            client.send(line);
        }
    }
    else
    {
        printf("Useage: %s userID pasword\n", argv[0]);
    }
}