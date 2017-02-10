#ifndef MUDUO_NET_CALLBACKS_H__
#define MUDUO_NET_CALLBACK_H__

#include <functional>
#include <memory>
#include "Timestamp.h"
#include "Buffer.h"

namespace muduo {
// All client visible callbacks go here.

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void ()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&,Buffer* buf,Timestamp)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&)> HighWaterMarkCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

}


#endif // MUDUO_NET_CALLBACK_H__
