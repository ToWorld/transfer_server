#ifndef MUDUO_BASE_CER_PACKET_H__
#define MUDUO_BASE_CER_PACKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include "copyable.h"
namespace muduo {
class CerPacket : public copyable{
public:
	const std::string getPacketAsString() const;
	const std::string getRandom() const;
	const std::string getCer() const;
	void setRand(std::string rand);
	void setCer(std::string cer);
	std::string encry();	
	std::string decry();
	void init();
private:
	std::string rand_;
	int length_;
	std::string cer_;
	
};
}
#endif // MUDUO_BASE_CER_PACKET__
