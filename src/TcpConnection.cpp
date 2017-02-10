#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"
#include "Timestamp.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "LogFile.h"
#include <string.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <errno.h>
#include <stdio.h>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop,const std::string& nameArg,int sockfd,const InetAddress& localAddr,const InetAddress& peerAddr)
 : loop_(loop),name_(nameArg),state_(kConnecting),socket_(new Socket(sockfd)),channel_(new Channel(loop,sockfd)),
   localAddr_(localAddr),peerAddr_(peerAddr) {
	LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << "this" << " fd=" << sockfd;
	//printf("into TcpConnection\n");
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
}

TcpConnection::~TcpConnection() {
	LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << "this" << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished() {
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->enableReading();

	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
	loop_->assertInLoopThread();
	assert(state_ == kConnected || state_ == kDisconnecting);
	setState(kDisconnected);
	channel_->disableAll();
	connectionCallback_(shared_from_this());

	loop_->removeChannel(channel_);
}

void TcpConnection::handleRead(Timestamp receiveTime) {
	int savedErrno = 0;
//	char buf[65536];
//	ssize_t n = ::read(channel_->fd(),buf,sizeof(buf));
	ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
	// FIXME: close connection if n == 0
	if (n > 0) {
		messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
	}
	else if (n == 0) {
		handleClose();
	}
	else {
		errno = savedErrno;
		LOG_SYSERR << "TcpConnection::handleRead";
		handleError();
	}
}

void TcpConnection::handleWrite() {
	loop_->assertInLoopThread();
	if (channel_->isWriting()) {
		ssize_t n = ::write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
		if (n > 0) {
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0) {
				channel_->disableWriting();
				if (writeCompleteCallback_) {
					loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
				}
				if (state_ == kDisconnecting) {
					shutdownInLoop();
				}
			}
			else {
				LOG_TRACE << "I am going to write more data";
			}
		}
		else {
			LOG_SYSERR << "TcpConnection::handleWrite";
		}
	}
	else {
		LOG_TRACE << "Connection is down, no more writing";
	}
}

void TcpConnection::handleClose() {
	loop_->assertInLoopThread();
	LOG_TRACE << "TcpConnection::handleclose state = " << state_;
	assert(state_ == kConnected || state_ == kDisconnecting);
	// we don't close fd, leave it to dtor,so we can find leaks easily
	channel_->disableAll();
	// must be the last line
	closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
	int err = sockets::getSocketError(channel_->fd());
	LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " "; // << strerror_tl(err);
}

void TcpConnection::shutdown() {
	// FIXME: use compare and swap
	if (state_ == kConnected) {
		setState(kDisconnecting);
		// FIXME: shared_from_this()?
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
	}
}

void TcpConnection::shutdownInLoop() {
	loop_->assertInLoopThread();
	if (!channel_->isWriting()) {
		// we are not writing
		socket_->shutdownWrite();
	}
}

void TcpConnection::setTcpNoDelay(bool on) {
	socket_->setTcpNoDelay(on);
}

void TcpConnection::send(const std::string& message) {
	if (state_ == kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(message);
		}
		else {
			loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,message));
		}
	}
}

void TcpConnection::sendInLoop(const std::string& message) {
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	// if no thing in output queue,trying writing directly
	if (outputBuffer_.readableBytes() == 0 && !channel_->isWriting()) {
		nwrote = ::write(channel_->fd(),message.data(),strlen(message.data()));
		if (nwrote >= 0) {
			if (static_cast<size_t>(nwrote) < message.size()) {
				LOG_TRACE << "I am going to write more data";
			}
			else if (writeCompleteCallback_) {
				loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
			}
		}
		else {
			LOG_SYSERR << "TcpConnection::sendInLoop";
		}
	}
	//std::cout << "nwrote==" << nwrote << std::endl;
	assert(nwrote >= 0);
	if (static_cast<size_t>(nwrote) < message.size()) {
		outputBuffer_.append(message.data()+nwrote,message.size()-nwrote);
		if (!channel_->isWriting()) {
			channel_->enableWriting();
		}
	}
}
