#include "CerPacket.h"
#include "AsyncLogging.h"
#include "Logging.h"
#include "Timestamp.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Channel.h"
#include "RegisterTable.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>

int kRollSize = 1*1000*1000;

muduo::AsyncLogging* g_asyncLog = NULL;

muduo::TcpServer* g_server;

muduo::CerPacket serPacket;

muduo::RegisterTable registerTable;

void asyncOutput(const char* msg,int len) {
	if (g_asyncLog == NULL) {
		printf("g_asyncLog is NULL\n");
	}
	g_asyncLog->append(msg,len);
}

void onConnection(const muduo::TcpConnectionPtr& conn) {
	if (conn->connected()) {	
		printf("onConnection(): tid=%d new connection [%s] from %s\n",muduo::CurrentThread::tid(),conn->name().c_str(),conn->peerAddress().toHostPort().c_str());

		std::string sendInfo = serPacket.getPacketAsString();
		conn->send(sendInfo);
	}
	else {
		printf("onConnection(): tid = %d connection [%s] is down\n",muduo::CurrentThread::tid(),conn->name().c_str());
	}
}

void onMessage(const muduo::TcpConnectionPtr& conn,muduo::Buffer* buf,muduo::Timestamp receiveTime) {
	std::string str = conn->getAsString();
	// register information
	registerTable.myRegister(str);

	muduo::CerPacket cliPacket1;
	muduo::CerPacket cliPacket2;

	if (registerTable.getValue(str) == 1) {
		registerTable.addValue(str);
		
		// first time get packet
		cliPacket1.init();
		std::string data_from_client = buf->retrieveAsString();
		char tmpRand[20] = {0};
		char tmpCer[20] = {0};
		data_from_client.copy(tmpRand,20,0);
		data_from_client.copy(tmpCer,data_from_client.length()-20,20);
		printf("data_from_client == %s\n",data_from_client.c_str());
		printf("tmpRand == %s\n",tmpRand);
		printf("tmpCer == %s\n",tmpCer);
		cliPacket1.setRand(std::string(tmpRand));
		cliPacket1.setCer(std::string(tmpCer));
		
		printf("before encry: cliPacket1.getRandom()==%s\n",cliPacket1.getRandom().c_str());
		printf("after encry: cliPacket1.encry() == %s\n",cliPacket1.encry().c_str());

		std::string sendInfo = cliPacket1.encry();
		conn->send(sendInfo);

		printf("done\n");
	}
	else if (registerTable.getValue(str) == 2){
		registerTable.addValue(str);
		cliPacket2.init();
		// second time get packet 
		std::string data_from_client = buf->retrieveAsString();
		printf("std::string data_from_client == %s\n",data_from_client.c_str());
		char tmpRand[21] = {0};
//		data_from_client.copy(tmpRand,20,0);
		memcpy(tmpRand,data_from_client.c_str(),20);
		printf("tmpRand == %s\n",tmpRand);
		cliPacket2.setRand(std::string(tmpRand));
		cliPacket2.setCer(serPacket.getCer());
		std::string getInfo = cliPacket2.decry();
		printf("getInfo == %s\n",getInfo.c_str());
		printf("serPacket.getRandom() == %s\n",serPacket.getRandom().c_str());
		if (strcmp(getInfo.c_str(),serPacket.getRandom().c_str())==0) {
			printf("identify success\n");
			conn->send(cliPacket1.encry());
		}
		else {
			// remove the connection
			g_server->removeConnection(conn);
			registerTable.myUnregister(str);
		}
	}
	else {
		printf("into here\n");

		printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s\n",muduo::CurrentThread::tid(),buf->readableBytes(),conn->name().c_str(),receiveTime.toFormattedString().c_str());
		//	auto connections = server.getConnections();
		auto connections = g_server->getConnections();
		// dispatcher 
		std::string message = buf->retrieveAsString();
		for (auto it = connections.begin(); it != connections.end(); ++it) {
			std::string s = (it->second)->getAsString();
			if ((it->second)!=conn) {
				if (registerTable.getValue(s)==0) {
					g_server->removeConnection(it->second);	
				}
				else {
		    	  	(*(it->second)).send(message);
				}
			}
			
	    }

	}
}

int main(int argc,char* argv[]) {
	printf("main(): pid = %d\n",getpid());
	/*
	{
		// set max virtual memory to 2GB
		size_t kOneGB = 1000*1024*1024;
		rlimit rl = {2*kOneGB,2*kOneGB};
		setrlimit(RLIMIT_AS,&rl);
	}*/
	
	serPacket.init();
	serPacket.setRand(std::string("cdefghijklmnopqrstuv"));
	serPacket.setCer(std::string("1"));

	char name[256];
	strncpy(name,argv[0],256);
	muduo::AsyncLogging log(::basename(name),kRollSize);
	muduo::Logger::setOutput(asyncOutput);
	log.start();
	g_asyncLog = &log;

	muduo::InetAddress listenAddr(9981);
	muduo::EventLoop loop;
	muduo::TcpServer server(&loop,listenAddr,"Server");
	g_server = &server;

	server.setConnectionCallback(onConnection);
	server.setMessageCallback(onMessage);
	if (argc > 1) {
		server.setThreadNum(atoi(argv[1]));
	}
	server.start();

	loop.loop();
//	bench(false);
}
