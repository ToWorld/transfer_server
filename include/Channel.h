#ifndef MUDUO_NET_CHANNEL_H__
#define MUDUO_NET_CHANNEL_H__

#include <functional>
#include "noncopyable.h"
//#include "EventLoop.h"
#include "Timestamp.h"

namespace muduo {
class EventLoop;

/// A selectable I/O channel.
/// 
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : public noncopyable {
public:
	typedef std::function<void ()> EventCallback;
	typedef std::function<void (Timestamp)> ReadEventCallback;
	Channel(EventLoop* loop,int fd);
	~Channel();
	/*
	void retset(Channel* Channel) {
		fd_ = Channel->fd();
		loop_ = Channel->loop_;
	}*/
	void handleEvent(Timestamp receiveTime);
	//void handleEvent();
	void setReadCallback(const ReadEventCallback& cb) {
		readCallback_ = cb;
	}
	void setWriteCallback(const EventCallback& cb) {
		writeCallback_ = cb;	
	}
	void setErrorCallback(const EventCallback& cb) {
		errorCallback_ = cb;
	}
	void setCloseCallback(const EventCallback& cb) {
		closeCallback_ = cb;
	}
	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; }
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	void enableReading() { events_ |= kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	bool isWriting() const {
		return events_ & kWriteEvent;
	}
	// for Poller
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }
	EventLoop* ownerLoop() { return loop_; }
/*	void reset(Channel* channel = NULL) {
		if (channel) {
			loop_ = channel->loop_;
			fd_ = channel->fd();	
		}
	}*/
private:
	void update();
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	bool eventHandling_;

	EventLoop* loop_;
	int fd_;
	int events_;
	int revents_;
	int index_;	// used by Poller

	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback errorCallback_;
	EventCallback closeCallback_;
};
}

#endif // MUDUO_NET_CHANNEL_H__

