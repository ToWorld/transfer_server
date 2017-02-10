#ifndef MUDUO_EXAMPLES_HUB_CODEC_H__
#define MUDUO_EXAMPLES_HUB_CODEC_H__

// internal header file

#include "Buffer.h"

namespace pubsub {
enum ParseResult {
	kError,kSuccess,kContinue,
};

ParseResult parseMessage(muduo::Buffer* buf,std::string& cmd,std::string& topic,std::string& content);
}


#endif // MUDUO_EXAMPLES_HUB_CODEC_H__
