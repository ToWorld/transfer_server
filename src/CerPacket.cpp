#include "CerPacket.h"

using namespace muduo;

const std::string CerPacket::getPacketAsString() const{
	std::string result = rand_ + cer_;
	return result;
}

std::string CerPacket::encry() {
	std::string result;
	int cnt = 0;
	for (auto i : rand_) {
		if (i != '\0') {
			result[cnt++] = i+1;
		}
	}
	return result;
}

std::string CerPacket::decry() {
	printf("before decry: rand_ == %s\n",rand_.c_str());
	std::string result;
	int cnt = 0;
	for (auto i : rand_)	 {
		if (i != '\0') {
			result[cnt++] = i-1;
		}
	}
	printf("after decry: result == %s\n",result.c_str());
	return result;
}

const std::string CerPacket::getRandom() const {
	return rand_;
}
const std::string CerPacket::getCer() const {
	return cer_;
}

void CerPacket::setRand(std::string random) {
	rand_ = random;
}

void CerPacket::setCer(std::string cer) {
	cer_ = cer;
	length_ = cer_.length();
}

void CerPacket::init() {
	length_ = 0;
	rand_.clear();
	cer_.clear();
}
