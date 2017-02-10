#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>

using namespace muduo;

Socket::~Socket() {
	sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
	sockets::bindOrDie(sockfd_,addr.getSockAddrInet());
}

void Socket::listen() {
	sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress* peeraddr) {
	//printf("into InetAddress::accept\n");
	//sleep(5);
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	int connfd = sockets::accept(sockfd_,&addr);
	if (connfd >= 0) {
		peeraddr->setSockAddrInet(addr);
	}
	return connfd;
}

void Socket::setReuseAddr(bool on) {
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	// FIXME CHECK
}

void Socket::shutdownWrite() {
	sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on) {
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(optval));
	// FIXME CHECK
}
