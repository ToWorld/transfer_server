#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <memory>

using namespace muduo;

EventLoopThread::EventLoopThread()
 : loop_(NULL),existing_(false),thread_(std::bind(&EventLoopThread::threadFunc,this)),mutex_(),cond_(mutex_)
{}

EventLoopThread::~EventLoopThread() {
	existing_ = false;
	loop_->quit();
	thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
	assert(!thread_.started());
	thread_.start();

	{
		MutexLockGuard lock(mutex_);
		while (loop_==NULL) {
			cond_.wait();
		}
	}
	return loop_;
}

void EventLoopThread::threadFunc() {
	EventLoop loop;

	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();
	}

	loop.loop();
	// assert(existing_);
}
