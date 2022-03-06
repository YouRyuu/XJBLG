#ifndef WS_CALLBACKS_H
#define WS_CALLBACKS_H
#include <memory>
#include <functional>
template<class T>
inline T* getPointer(const std::shared_ptr<T>& ptr)
{
    return ptr.get();
}

template<class T>
inline T* getPointer(const std::unique_ptr<T>& ptr)
{
    return ptr.get();
}

class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void (const TcpConnectionPtr&, Buffer*, int)> MessageCallback;
void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, int);
#endif