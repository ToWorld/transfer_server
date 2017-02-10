#ifndef MUDUO_NET_TIMER_H__
#define MUDUO_NET_TIMER_H__

#include "noncopyable.h"
#include "Timestamp.h"
#include "Callbacks.h"

namespace muduo {
///
/// Internal class for timer event.
///
class Timer : public noncopyable {
public:
	Timer(const TimerCallback& cb,Timestamp when,double interval)
	 : callback_(cb),expiration_(when),interval_(interval),repeat_(interval>0) {}
	void run() const { callback_(); }
	Timestamp expiration() const { return expiration_; }
	bool repeat() const { return repeat_; }
	void restart(Timestamp now);
private:
	const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
};
}

#endif // MUDUO_NET_TIMER_H__
