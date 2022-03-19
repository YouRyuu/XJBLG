objs = Acceptor.o Buffer.o Channel.o CountDownLatch.o CurrentThread.o \
Epoll.o Eventloop.o EventLoopThread.o EventLoopThreadPool.o HttpContext.o \
HttpResponse.o HttpServer.o InetAddress.o main.o Socket.o SocketsOps.o \
TcpConnection.o TcpServer.o Thread.o ThreadPool.o

server:$(objs)
	g++ -o server $(objs) -std=c++11 -lpthread

main.o:InetAddress.h Socket.h Eventloop.h Acceptor.h TcpConnection.h TcpServer.h Buffer.h StringPiece.h ThreadPool.h CurrentThread.h HttpResponse.h HttpServer.h HttpRequest.h
Acceptor.o:Channel.h Socket.h Acceptor.h Eventloop.h InetAddress.h SocketsOps.h
Buffer.o:StringPiece.h Buffer.h SocketsOps.h
Channel.o:Channel.h Eventloop.h
CountDownLatch.o:Mutex.h Condition.h CountDownLatch.h
CurrentThread.o:CurrentThread.h
Epoll.o:Epoll.h Channel.h Eventloop.h
Eventloop.o:CurrentThread.h Mutex.h Eventloop.h Channel.h Epoll.h
EventLoopThread.o:Mutex.h Condition.h Thread.h Eventloop.h EventLoopThread.h
EventLoopThreadPool.o :EventLoopThreadPool.h EventLoopThread.h Eventloop.h
HttpContext.o :HttpRequest.h HttpContext.h Buffer.h
HttpResponse.o :HttpResponse.h Buffer.h
HttpServer.o :TcpServer.h Callbacks.h
InetAddress.o :InetAddress.h
Socket.o :Socket.h InetAddress.h SocketsOps.h
SocketsOps.o :SocketsOps.h
TcpConnection.o : InetAddress.h Callbacks.h Buffer.h StringPiece.h HttpContext.h TcpConnection.h Channel.h Eventloop.h Socket.h SocketsOps.h
TcpServer.o :TcpConnection.h TcpServer.h Acceptor.h Eventloop.h SocketsOps.h EventLoopThreadPool.h
Thread.o : CountDownLatch.h Thread.h CurrentThread.h
ThreadPool.o:Condition.h Mutex.h Thread.h ThreadPool.h

.PHONY : clean
clean:
	rm $(objs)