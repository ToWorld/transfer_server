#ifndef MUDUO_NET_EVENTLOOP_H__
#define MUDUO_NET_EVENTLOOP_H__

#include "noncopyable.h"
#include "CurrentThread.h"
//#include "Poller.h"
//#include "Channel.h"
#include "Thread.h"
#include "Callbacks.h"
#include "TimerId.h"
#include "Mutex.h"

#include <pthread.h>
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include <memory>
#include <functional>
#include <iostream>

namespace muduo {
class Channel;
class Poller;
class TimerQueue;
class EventLoop : public noncopyable {
public:
	typedef std::function<void()> Functor;
	EventLoop();
	~EventLoop();
	
	void loop();
	void quit();

	///
	/// Time when poll returns, usually means data arrival.
	///
	Timestamp pollReturnTime() const { return pollReturnTime_; }
	
	/// Runs callback immediately in the loop thread.
	/// it wakes up the loop, and run the cb.
	/// If in the same loop thread, cb is run within the function.
	/// Safe to call from other threads.
	void runInLoop(const Functor& cb);

	/// Queues callback in the loop thread.
	/// Runs after finish polling
	/// Safe to call from other threads.
	void queueInLoop(const Functor& cb);

//	void updateChannel(Channel* channel);
	bool isInLoopThread() const {
		return threadId_ == CurrentThread::tid();
	}
	void assertInLoopThread() {
		if (!isInLoopThread()) {
			abortNotInLoopThread();
		}
	}

	// timers

	/// 
	/// Runs callback at 'time'.
	///
	TimerId runAt(const Timestamp& time,const TimerCallback& cb);
	///
	/// Runs callback after @c delay seconds.
	///
	TimerId runAfter(double delay,const TimerCallback& cb);
	///
	/// Runs callback every @c interval seconds.
	///
	TimerId runEvery(double interval,const TimerCallback& cb);
	
	// void cancel (TimerId timerId);

	// internal use only
	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	
private:
	void abortNotInLoopThread();
	void handleRead();	// wake up
	void doPendingFunctors();
	typedef std::vector<Channel*>	 ChannelList;

	bool looping_;	/* atomic */
	bool quit_;	/* atomic */
	bool callingPendingFunctors_;	/* atomic */
	const pid_t threadId_;
	Timestamp pollReturnTime_;
	std::unique_ptr<Poller> poller_;
	std::unique_ptr<TimerQueue> timerQueue_;
	int wakeupFd_;
	// unlike in TimerQueue, which is an internal class,
	// we don't expose Channel to client.
	std::unique_ptr<Channel> wakeupChannel_;
	ChannelList activeChannels_;
	MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;	// @GuardedBy mutex_
};
}

#endif // MUDUO_NET_EVENTLOOP_H__
