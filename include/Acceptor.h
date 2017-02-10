#ifndef MUDUO_NET_ACCEPTOR_H__
#define MUDUO_NET_ACCEPTOR_H__

#include <functional>
#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h"

namespace muduo {
class EventLoop;
class InetAddress;

/// 
/// Acceptor of incoming TCP connections.
///
class Acceptor : public noncopyable {
public:
	typedef std::function<void (int sockfd,const InetAddress&)> NewConnectionCallback;
	Acceptor(EventLoop* loop,const InetAddress& listenAddr);
	void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
	bool listening() const { return listening_; }
	void listen();
private:
	void handleRead();

	EventLoop* loop_;
	Socket acceptSocket_;
	Channel acceptChannel_;
	NewConnectionCallback newConnectionCallback_;
	bool listening_;
	
};

}

#endif // MUDUO_NET_ACCEPTOR_H__
