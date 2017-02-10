#ifndef MUDUO_NET_TCPSERVER_H__
#define MUDUO_NET_TCPSERVER_H__

#include "Callbacks.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

#include <map>
#include <string>
#include "noncopyable.h"
#include <functional>
#include <memory>

namespace muduo {
class Acceptor;
class EventLoop;

class TcpServer : public noncopyable {
public:
	TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg="muduo");
	~TcpServer();	// force out-line dtor, for std::unique_ptr members

	/// Set the number of threads for handling input.
	///
	/// Always accepts new connection in loop's thread.
	/// Must be called before @c start
	/// @param numThreads
	/// - 0 means all I/O in loop's thread, no thread will be created.
	///		this the default value.
	///	- 1 means all I/O in another thread.
	/// - N means a thread pool with N threads, new connections
	///     are assigned on a round-robin basis.
	void setThreadNum(int numThreads);
	/// Starts the server if it's not listening
	///
	/// It's harmless to call it multiple times.
	/// Thread safe.
	void start();

	/// Set connection callback.
	/// Not thread safe.
	void setConnectionCallback(const ConnectionCallback& cb) {
		connectionCallback_ = cb;
	}

	/// Set message callback.
	/// Not thread safe.
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
	//void removeConnection(const TcpConnectionPtr& conn);
	std::map<std::string,TcpConnectionPtr>& getConnections(){
		return connections_;	
	}
	void removeConnection(const TcpConnectionPtr& conn);
private:
	/// Not thread safe, but in loop
	void newConnection(int sockfd,const InetAddress& peerAddr);
	/// Thread safe.
	//void removeConnection(const TcpConnectionPtr& conn);
	/// Not Thread safe, but in loop
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<std::string,TcpConnectionPtr> ConnectionMap;
	
	EventLoop* loop_;	// the acceptor loop
	const std::string name_;
//	std::unique_ptr<Acceptor> acceptor_;	// avoid revealing Acceptor
	Acceptor* acceptor_;
	EventLoopThreadPool* threadPool_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;	
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;
	bool started_;
	int nextConnId_;	// always in loop thread
	ConnectionMap connections_;	
	//std::set<TcpConnectionPtr> connections_;
};
}

#endif // MUDUO_NET_TCPSERVER_H__
