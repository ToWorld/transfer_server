#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <unistd.h>
#include <functional>

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop,const InetAddress& listenAddr) 
 : loop_(loop),acceptSocket_(sockets::createNonblockingOrDie()),
   acceptChannel_(loop,acceptSocket_.fd()),listening_(false) {
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

void Acceptor::listen() {
	loop_->assertInLoopThread();
	listening_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
	loop_->assertInLoopThread();
	InetAddress peerAddr(0);
	// FIXME loop until no more
//	printf("into Acceptor::handleRead\n");
//	sleep(5);
	int connfd = acceptSocket_.accept(&peerAddr);
	//printf("get connfd = %d where it is in Acceptor::handleRead\n",connfd);
	if (connfd >= 0) {
		if (newConnectionCallback_) {
			newConnectionCallback_(connfd,peerAddr);
		}
		else {
			sockets::close(connfd);
		}
	}
}
