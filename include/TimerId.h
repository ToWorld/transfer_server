#ifndef MUDUO_NET_TIMERID_H__
#define MUDUO_NET_TIMERID_H__

#include "copyable.h"
#include "Callbacks.h"

namespace muduo {
class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public copyable {
public:
	TimerId(Timer* timer=NULL) : value_(timer) {}
	// default copy-ctor,dtor and assignment are okay
private:
	Timer* value_;
};


}

#endif // MUDUO_NET_TIMERID_H__
