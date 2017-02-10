#ifndef MUDUO_NET_POLLER_H__
#define MUDUO_NET_POLLER_H__

#include <map>
#include <vector>

#include "Timestamp.h"
#include "EventLoop.h"
#include "noncopyable.h"

struct pollfd;

namespace muduo {
class Channel;
/// IO Multiplexing with poll(2)
///
/// This class doesn't own the Channel objects.
class Poller : public noncopyable {
public:
	typedef std::vector<Channel*> ChannelList;
	Poller(EventLoop* loop);
	~Poller();

	/// Polls the I/O events.
	/// Must be called in the loop thread.
	Timestamp poll(int timeoutMs,ChannelList* activeChannels);

	/// Changes the interested I/O events.
	/// Must be called in the loop thread.
	void updateChannel(Channel* channel);
	void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }
	void removeChannel(Channel* channel);
private:
	void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
	typedef std::vector<struct pollfd> PollFdList;
	typedef std::map<int,Channel*> ChannelMap;

	EventLoop* ownerLoop_;
	PollFdList pollfds_;
	ChannelMap channels_;
};
}

#endif // MUDUO_NET_POLLER_H__
