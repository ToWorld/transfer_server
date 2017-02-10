#ifndef MUDUO_NET_EVENTLOOPTHREAD_H__
#define MUDUO_NET_EVENTLOOPTHREAD_H__

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"

namespace muduo {
class EventLoop;
class EventLoopThread : public noncopyable {
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();
private:
	void threadFunc();
		
	EventLoop* loop_;
	bool existing_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
};
}

#endif // MUDUO_NET_EVENTLOOPTHREAD_H__
