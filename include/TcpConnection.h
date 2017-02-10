#ifndef MUDUO_NET_TCPCONNECTION_H__
#define MUDUO_NET_TCPCONNECTION_H__

#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "Buffer.h"
#include <set>
#include <string>
#include <functional>
#include <memory>

namespace muduo {
class Channel;
class EventLoop;
class Socket;

class TcpConnection : public noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
	/// Constructs a TcpConnection with a connected sockfd
	///
	/// User should not create this object.
	TcpConnection(EventLoop* loop, const std::string& name,int sockfd,const InetAddress& localAddr,const InetAddress& peerAddr);
	~TcpConnection();
	EventLoop* getLoop() const { return loop_; }
	const std::string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	InetAddress& peerAddress() { return peerAddr_; }
	bool connected() const { return state_ == kConnected; }
	std::string getAsString() {
		return 	peerAddress().getAsString();
	}

	// void send(const void* message,size_t len);
	// Thread safe
	void send(const std::string& message);
	// Thread safe
	void shutdown();
	void setTcpNoDelay(bool on);

	void setContext(const std::set<std::string>& context) {
		context_ = context;
	}
	const std::set<std::string>& getContext() {
		return context_;
	}
	std::set<std::string>* getMutableContext() {
		return &context_;
	}

	void setConnectionCallback(const ConnectionCallback& cb) {
		connectionCallback_ = cb;
	}
	void setMessageCallback(const MessageCallback& cb) {
		messageCallback_ = cb;
	}
	void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
		writeCompleteCallback_ = cb;
	}
	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
		highWaterMarkCallback_ = cb;
	}
	void setCloseCallback(const CloseCallback& cb) {
		closeCallback_ = cb;
	}

	// Internal use only.

	// called when TcpServer accepts a new connection
	void connectEstablished();	// should be called only once
	// called when TcpServer has removed me from its map
	void connectDestroyed();	// should be called only once
private:
	enum StateE { kConnecting, kConnected, kDisconnecting,kDisconnected,};	

	void setState(StateE s) { state_ = s; }
	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();
	void sendInLoop(const std::string& message);
	void shutdownInLoop();
	

	EventLoop* loop_;
	std::string name_;
	StateE state_;
	// we don't expose those classes to client.
	std::unique_ptr<Socket> socket_;
//	std::unique_ptr<Channel>  channel_;
	Channel* channel_;
	InetAddress localAddr_;
	InetAddress peerAddr_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
	std::set<std::string> context_;
};
//	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}

#endif // MUDUO_NET_TCPCONNECTION_H__
