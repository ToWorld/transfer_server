#include "Logging.h"
#include "AsyncLogging.h"
#include "LogFile.h"
#include "CurrentThread.h"
#include "Timestamp.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

namespace muduo {
__thread char t_errnobuf[512];
__thread char t_time[32];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno) {
	return strerror_r(savedErrno,t_errnobuf,sizeof(t_errnobuf));
}

Logger::LogLevel initLogLevel() {
	if (::getenv("MUDUO_LOG_TRACE")) {
		return Logger::TRACE;
	}
	else if (::getenv("MUDUO_LOG_DEBUG")) {
		return Logger::DEBUG;
	}
	else {
		return Logger::INFO;
	}
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL"
};

// helper class for known string length at compile time
class T {
public:
	T(const char* str,unsigned len) : str_(str),len_(len) {
		//assert(strlen(str)==len_);
		;
	}
	const char* str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream& s,T v) {
	s.append(v.str_,v.len_);
	return s;
}

inline LogStream& operator<<(LogStream& s,const Logger::SourceFile& v) {
	s.append(v.data_,v.size_);
	return s;
}

void defaultOutput(const char* msg,int len) {
	printf("into defaultOutput-------------\n");
	size_t n = fwrite(msg,1,len,stdout);
}

void defaultFlush() {
	printf("into defaultFlush--------------\n");
	fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
}

using namespace muduo;

Logger::Impl::Impl(LogLevel level,int savedErrno,const SourceFile& file,int line)
 : time_(Timestamp::now()),stream_(),level_(level),line_(line),basename_(file) {
	formatTime();
	CurrentThread::tid();
//	stream_ << T(CurrentThread::tidString(),6);
//	stream_ << T(LogLevelName[level],6);
//	printf("currentThread::tidString() == %s\n",CurrentThread::tidString());
//	printf("%s\n",LogLevelName[level]);
	stream_ << CurrentThread::tidString();
	stream_ << LogLevelName[level];
//	printf("%s\n",stream_.buffer_.data_);
//	printf("into Logger::Impl::Impl\n");
	if (savedErrno != 0) {
		stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
	}
}

void Logger::Impl::formatTime() {
	int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
	int microseconds = static_cast<time_t>(microSecondsSinceEpoch % 1000000);
	if (seconds != t_lastSecond) {
		t_lastSecond = seconds;
		struct tm tm_time;	
		::gmtime_r(&seconds,&tm_time);
	
		int len = snprintf(t_time,sizeof(t_time),"%4d%02d%02d %02d:%02d:%02d",tm_time.tm_year + 1900,tm_time.tm_mon+1,tm_time.tm_mday,tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
		assert(len == 17);
	}
	Fmt us(".%06dZ ",microseconds);
	assert(us.length() == 9);
	stream_ << T(t_time,17) << T(us.data(),9);
}

void Logger::Impl::finish() {
	stream_ << " - " << basename_ << ":" << line_ << '\n';
}

Logger::Logger(SourceFile file,int line) : impl_(INFO,0,file,line) {}
Logger::Logger(SourceFile file,int line,LogLevel level,const char* func) : impl_(level,0,file,line) {
	impl_.stream_ << func << ' ';
	//printf("%s\n",impl_.stream_.buffer_.data_);
}
Logger::Logger(SourceFile file,int line,LogLevel level) : impl_(level,0,file,line) {}
Logger::Logger(SourceFile file,int line,bool toAbort) : impl_(toAbort ? FATAL : ERROR,errno,file,line) {}
Logger::~Logger() {
	//printf("into ~Logger\n");
	//sleep(5);
	impl_.finish();
//	printf("impl_.stream_ = %s\n",impl_.stream_.buffer_.data());
	//printf("after impl_.finish\n");
	const LogStream::Buffer& buf(stream().buffer());	
//	printf("const LogStream::Buffer& buf = %s\n",buf.data());
	g_output(buf.data(),buf.length());
//	printf("after g_output\n");
	//sleep(5);
//	printf("impl_.stream_ = %s\n",impl_.stream_.buffer_.data());
	if (impl_.level_ == FATAL) {
		printf("impl_.level is FATAL\n");
	//	sleep(10);
		g_flush();
//		printf("impl_.stream_ = %s\n",impl_.stream_.buffer_.data());
//		printf("after g_flush\n");
		//abort();
	}
}

void Logger::setLogLevel(Logger::LogLevel level) {
	g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) {
	g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
	g_flush = flush;
}
