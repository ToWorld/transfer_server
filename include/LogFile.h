#ifndef MUDUO_BASE_LOGFILE_H__
#define MUDUO_BASE_LOGFILE_H__

#include "Mutex.h"
#include "noncopyable.h"
#include <functional>
#include <memory>

namespace muduo {
class LogFile : public noncopyable {
public:
	LogFile(const std::string& basename,size_t rollSize,bool threadSafe = true,int flushInterval = 3);
	~LogFile();
	
	void append(const char* logline,int len);
	void flush();
	
	/*
	void reset(LogFile* p) {
		assert(p != NULL);
		
	}*/

private:
	void append_unlocked(const char* logline,int len);
	
	static std::string getLogFileName(const std::string& basename,time_t* now);
	void rollFile();

	const std::string basename_;
	const size_t rollSize_;
	const int flushInterval_;

	int count_;
	MutexLock* mutex_;
	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;
	class File;
	File* file_;

	const static int kCheckTimeRoll_ = 1024;
	const static int kRollPerSeconds_ = 60*60*24;
	
};
}

#endif // MUDUO_BASE_LOGFILE_H__
