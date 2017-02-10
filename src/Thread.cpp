#include "CurrentThread.h"
#include "Thread.h"
#include "Atomic.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "LogFile.h"
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <string>
#include <stdio.h>

namespace muduo {
namespace CurrentThread {
	__thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	__thread const char* t_threadName = "unknown";
	
}
namespace detail {
pid_t gettid() {
	return static_cast<pid_t>(::syscall(SYS_gettid));
}
}
}

using namespace muduo;

void CurrentThread::cacheTid() {
	if (t_cachedTid == 0) {
		t_cachedTid = detail::gettid();
		int n = snprintf(t_tidString,sizeof(t_tidString),"%5d",t_cachedTid);
		assert(n == 5);
		//printf("n==%d\n",n);
	}
}

bool CurrentThread::isMainThread() {
	return tid() == ::getpid();
}
AtomicInt32 Thread::numCreated_;
Thread::Thread(const ThreadFunc& func,const std::string& n) : started_(false),threadId_(0),tid_(0),func_(func),name_(n) {
	numCreated_.increment();	
}
Thread::~Thread() {}
void Thread::start() {
	assert(!started_);
	started_ = true;
	errno = pthread_create(&threadId_,NULL,&startThread,this);
	if (errno != 0) {
		LOG_SYSERR << "Failed in pthread_create";
	}
}

int Thread::join() {
	assert(started_);
	return pthread_join(threadId_,NULL);
}
	
void* Thread::startThread(void* obj) {
	Thread* thread = static_cast<Thread*>(obj);
	thread->runInThread();
	return NULL;
}

void Thread::runInThread() {
	tid_ = CurrentThread::tid();
	muduo::CurrentThread::t_threadName = name_.c_str();
	func_();
	muduo::CurrentThread::t_threadName = "finished";
}
