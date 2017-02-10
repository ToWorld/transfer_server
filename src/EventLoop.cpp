#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "Timer.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "LogFile.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <sys/types.h>
#include <sys/timerfd.h>
//#include <poll.h>
#include <assert.h>
#include <iostream>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd() {
	int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0) {
		LOG_SYSERR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

class IgnoreSigPipe {
public:
	IgnoreSigPipe() {
		::signal(SIGPIPE,SIG_IGN);
	}
};

IgnoreSigPipe initObj;

EventLoop::EventLoop() : looping_(false),quit_(false),callingPendingFunctors_(false),threadId_(CurrentThread::tid()),poller_(new Poller(this)),timerQueue_(new TimerQueue(this)),wakeupFd_(createEventfd()),wakeupChannel_(new Channel(this,wakeupFd_)) {
	//printf("into eventloop()\n");
	LOG_TRACE << "EventLoop created " << "this " << " in thread " << threadId_;
	if (t_loopInThisThread) {
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
	}
	else {
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
	// we are always reading the wakeupfd
	wakeupChannel_->enableReading();
	//printf("outof eventloop()\n");
}
EventLoop::~EventLoop() {
	assert(!looping_);
	::close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void EventLoop::abortNotInLoopThread() {
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << "this" << " was created in threadId_ = " << threadId_
			<< ", current thread id = " << CurrentThread::tid();
}

void EventLoop::loop() {
	LOG_TRACE << "into loop()";
	// first time
	assert(!looping_);	// if !looping_ == true --> looping_ == false ----- initial state
	assertInLoopThread();	// loop must run in the io thread
	looping_ = true;
	quit_ = false;
	int cnt = 0;
	while (!quit_) {
		//printf("%d\n",cnt++);
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
		for (ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end(); ++it) {
			(*it)->handleEvent(pollReturnTime_);
		}
		doPendingFunctors();
	}

//	::poll(NULL,0,5*1000);
	LOG_TRACE << "EventLoop " << "this" << " stop looping";
	looping_ = false;
}

void EventLoop::quit() {
	quit_ = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::updateChannel(Channel* channel) {
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->removeChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp& time,const TimerCallback& cb) {
	return timerQueue_->addTimer(cb,time,0.0);
}
TimerId EventLoop::runAfter(double delay,const TimerCallback& cb) {
	Timestamp time(addTime(Timestamp::now(),delay));
	return runAt(time,cb);
}
TimerId EventLoop::runEvery(double interval,const TimerCallback& cb) {
	Timestamp time(addTime(Timestamp::now(),interval));
	return timerQueue_->addTimer(cb,time,interval);
}
void EventLoop::queueInLoop(const Functor& cb) {
	{
	MutexLockGuard lock(mutex_);
	pendingFunctors_.push_back(cb);
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}
void EventLoop::runInLoop(const Functor& cb) {
	if (isInLoopThread()) {
		//printf("isInLoopThread\n");
		cb();
	}
	else {
		queueInLoop(cb);
	}
}
void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
	MutexLockGuard lock(mutex_);
	functors.swap(pendingFunctors_);
	}
	for (size_t i = 0; i < functors.size(); ++i) {
		functors[i]();
	}
	callingPendingFunctors_ = false;
}
void EventLoop::wakeup() {
	uint64_t one = 1;
	ssize_t n = ::write(wakeupFd_,&one,sizeof(one));
	if (n != sizeof(one)) {
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::handleRead() {
	uint64_t one = 1;
	ssize_t n = ::read(wakeupFd_,&one,sizeof(one));
	if (n != sizeof(one)) {
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
}
