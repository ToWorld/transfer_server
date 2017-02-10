#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H__
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H__

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"

#include <vector>
#include <functional>
#include <memory>
#include "noncopyable.h"

namespace muduo {
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public noncopyable {
public:
	EventLoopThreadPool(EventLoop* baseLoop);
	~EventLoopThreadPool();
	void setThreadNum(int numThreads) {
		numThreads_ = numThreads;
	}
	void start();
	EventLoop* getNextLoop();
private:
	EventLoop* baseLoop_;
	bool started_;
	int numThreads_;
	int next_;	// always in loop thread
	std::vector<EventLoopThread*> threads_;
	std::vector<EventLoop*> loops_;
};
}

#endif // MUDUO_NET_EVENTLOOPTHREADPOOL_H__
