#ifndef MUDUO_BASE_ASYNCLOGGING_H__
#define MUDUO_BASE_ASYNCLOGGING_H__

#include "LogStream.h"

//#include "BlockingQueue.h"
//#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"

#include <vector>
#include <functional>
#include <memory>

namespace muduo {
class AsyncLogging : public noncopyable {
public:
	typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
//	typedef std::unique_ptr<Buffer> BufferPtr;
	typedef Buffer* BufferPtr;
	typedef std::vector<BufferPtr> BufferVector;
	
	
	AsyncLogging(const std::string& basename,size_t rollSize,int flushInterval = 3)
	 :	flushInterval_(flushInterval),running_(false),basename_(basename),rollSize_(rollSize),
		thread_(std::bind(&AsyncLogging::threadFunc,this),"Logging"),latch_(1),
		mutex_(),cond_(mutex_),currentBuffer_(new Buffer),nextBuffer_(new Buffer),buffers_() {
		currentBuffer_->bzero();
		nextBuffer_->bzero();
		buffers_.reserve(16);
	}

	~AsyncLogging() {
		if (running_) {
			stop();
		}
	}
	
	void append(const char* logline,int len);

	void start() {
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop() {
		running_ = false;
		cond_.notify();
		thread_.join();
	}
private:
	void threadFunc();

	const int flushInterval_;
	bool running_;
	std::string basename_;
	size_t rollSize_;
	muduo::Thread thread_;
	muduo::CountDownLatch latch_;
	muduo::MutexLock mutex_;
	muduo::Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};
}


#endif // MUDUO_BASE_ASYNCLOGGING_H__
