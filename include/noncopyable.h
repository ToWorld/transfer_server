#ifndef MUDUO_BASE_NONCOPYABLE_H__
#define MUDUO_BASE_NONCOPYABLE_H__

class noncopyable {
public:
	noncopyable(){}
	~noncopyable(){}
private:
	noncopyable(const noncopyable&){}
	const noncopyable& operator=(const noncopyable&){}
};

#endif // MUDUO_BASE_NONCOPYABLE_H__
