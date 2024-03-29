#include "Condition.h"
#include <errno.h>
#include <time.h>

// returns true if time out, false otherwise.
bool muduo::Condition::waitForSeconds(int seconds) {
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME,&abstime);
	abstime.tv_sec += seconds;
	return ETIMEDOUT == pthread_cond_timedwait(&pcond_,mutex_.getPthreadMutex(),&abstime);
}
