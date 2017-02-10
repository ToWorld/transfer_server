#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "LogFile.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <stdio.h>	// snprintf

using namespace muduo;

TcpServer::TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg)
 : loop_(loop),name_(nameArg),acceptor_(new Acceptor(loop,listenAddr)),
   threadPool_(new EventLoopThreadPool(loop)),started_(false),nextConnId_(1) {
	//printf("into tcpserver()\n");
	acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
	if (!started_) {
		started_ = true;
		threadPool_->start();
	}
	if (!acceptor_->listening()) {
		loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_));
	}
}

void TcpServer::setThreadNum(int numThreads) {
	assert(0 <= numThreads);
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr) {
	//printf("into TcpServer::newConnection(int sockfd,const InetAddress& peerAddr)\n");
	loop_->assertInLoopThread();
	//printf("after loop_->assertInLoopThread\n");
	
	char buf[32];
	snprintf(buf,sizeof(buf),"#%d",nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;
//	LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection [" << connName << "] from " << peerAddr.toHostPort();
	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	EventLoop* ioLoop = threadPool_->getNextLoop();
	//printf("after threadPool_->getNextLoop\n");
	TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,sockfd,localAddr,peerAddr));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));	
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
	// FIXME: unsafe
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
	loop_->assertInLoopThread();	
	LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
	size_t n = connections_.erase(conn->name());
	assert(n==1);
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
}
