#ifndef MUDUO_BASE_THREAD_H__
#define MUDUO_BASE_THREAD_H__

#include "Atomic.h"
#include <string>
#include <functional>
#include "noncopyable.h"
#include <pthread.h>

namespace muduo {
class Thread : public noncopyable {
public:
	typedef std::function<void ()> ThreadFunc;
	explicit Thread(const ThreadFunc&,const std::string& name = std::string());
	~Thread();

	void start();
	int join();	// return pthread_join()

	bool started() const { return started_; }
	pid_t tid() const { return tid_; }
	const std::string& name() const { return name_; }
	
	static int numCreated() { return numCreated_.get(); }
private:
	static void* startThread(void* thread);
	void runInThread();

	bool started_;
	pthread_t threadId_;
	pid_t tid_;
	ThreadFunc func_;
	std::string name_;

	static AtomicInt32 numCreated_;
};
}

#endif // MUDUO_BASE_THREAD_H__
