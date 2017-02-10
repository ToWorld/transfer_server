#ifndef MUDUO_BASE_MUTEX_H__
#define MUDUO_BASE_MUTEX_H__

#include "noncopyable.h"
#include "CurrentThread.h"
#include <assert.h>
#include <pthread.h>

namespace muduo {
class MutexLock : public noncopyable {
public:
	MutexLock() : holder_(0) {
		int ret = pthread_mutex_init(&mutex_,NULL);
		assert(ret == 0);
	}
	~MutexLock() {
		assert(holder_ == 0);
		int ret = pthread_mutex_destroy(&mutex_);
		assert(ret == 0);
	}
	bool isLockedByThisThread() {
		return holder_ == CurrentThread::tid();
	}
	void assertLocked() {
		assert(isLockedByThisThread());
	}
	// internal usage
	void lock() {
		pthread_mutex_lock(&mutex_);
		holder_ = CurrentThread::tid();
	}
	void unlock() {
		holder_ = 0;
		pthread_mutex_unlock(&mutex_);	
	}
	pthread_mutex_t* getPthreadMutex() {
		return &mutex_;
	}
private:
	pthread_mutex_t mutex_;
	pid_t holder_;
};

class MutexLockGuard : public noncopyable {
public:
	explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) {
		mutex_.lock();
	}
	~MutexLockGuard() {
		mutex_.unlock();
	}
private:
	MutexLock& mutex_;
};
}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif // MUDUO_BASE_MUTEX_H__
