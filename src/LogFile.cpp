#include "LogFile.h"
#include "Logging.h"
#include "AsyncLogging.h"
#include "ProcessInfo.h"
#include "noncopyable.h"

#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

// not thread safe
class LogFile::File : public noncopyable {
public:
	explicit File(const std::string& filename) : fp_(::fopen(filename.data(),"ae")),writtenBytes_(0) {
		assert(fp_);
		::setbuffer(fp_,buffer_,sizeof(buffer_));
	}
	~File() {
		::fclose(fp_);
	}

//	typedef std::unique_ptr<File> this_type;
//	void reset(File* newfile = 0) {
//		this_type(newfile).swap(*this);
//		swap(*this,new);
//		std::unique_ptr<File,Deleter>(newfile).swap(std::unique_ptr<File,Deleter>(*this));
//		newfile = std::move(this);
//		this = NULL;
		
		//fp_ = newfile;
//		fp_ = newfile->fp_;
//		bzero(buffer_,sizeof(buffer_));
//		writtenBytes_ = 0;
//	}
	void append(const char* logline,const size_t len) {
		//printf("LogFile::File append\n");
		size_t n = write(logline,len);
		size_t remain = len - n;
		while (remain > 0) {
			size_t x = write(logline + n,remain);
			if (x == 0) {
				int err = ferror(fp_);
				if (err) {
					fprintf(stderr,"LogFile::File::append() failed %s\n",strerror_tl(err));
				}
				break;
			}
			n += x;
			remain = len - n;
		}

		writtenBytes_ += len;
	}

	void flush() {
		::fflush(fp_);
		/*
		if (fp_ == NULL) {
			printf("fp_---------------is NULL\n");
		}
		else {
			printf("fp_---------------is not NULL\n");
		}*/
	}
	size_t writtenBytes() const {
		return writtenBytes_;
	}
private:
	size_t write(const char* logline,size_t len) {
		if (fp_ == NULL) {
			printf("fp_ is NULL\n");
			sleep(5);
		}
#undef fwrite_unlocked
		assert(fp_);
		//printf("after ::fwrite_unlocked: logline = %s\n",logline);
//		return ::fwrite_unlocked(logline,1,len,fp_);
		return ::fwrite(logline,1,len,fp_);
		//assert(fp_);
		//printf("after ::fwrite_unlocked: logline = %s\n",logline);
	}
	FILE* fp_;
	char buffer_[64*1024];
	size_t writtenBytes_;
};

LogFile::LogFile(const std::string& basename,size_t rollSize,bool threadSafe,int flushInterval)
 : basename_(basename),rollSize_(rollSize),flushInterval_(flushInterval),count_(0),mutex_(threadSafe ? new MutexLock : NULL),startOfPeriod_(0),
	lastRoll_(0),lastFlush_(0) {
	assert(basename.find('/') == std::string::npos);	
	rollFile();
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline,int len) {
//	printf("into LogFile append\n");
	if (mutex_) {
		MutexLockGuard lock(*mutex_);
//		printf("into LogFile append\n");
		append_unlocked(logline,len);
		//printf("%s\n",logline);
	}
	else {
		append_unlocked(logline,len);
	}
	//printf("outof LogFile append\n");
}

void LogFile::flush() {
	if (mutex_) {
		MutexLockGuard lock(*mutex_);
		file_->flush();
	}
	else {
		file_->flush();	
	}
}

void LogFile::append_unlocked(const char* logline,int len) {
//	printf("Logfile append_unlocked\n");
	file_->append(logline,len);
	//printf("LogFile::append_unlocked:logline==%s\n",logline);

	if (file_->writtenBytes() > rollSize_) {
		rollFile();
	}
	else {
		if (count_ > kCheckTimeRoll_) {
			count_ = 0;
			time_t now = ::time(NULL);
			time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
			if (thisPeriod_ != startOfPeriod_) {	
				rollFile();		
			}
			else if (now - lastFlush_ > flushInterval_) {
				lastFlush_ = now;
				file_->flush();
			}
		}
		else {
			++count_;
		}
	}
}

void LogFile::rollFile() {
	time_t now = 0;
	std::string filename = getLogFileName(basename_,&now);	
	time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

	if (now > lastRoll_) {
		lastRoll_ = now;
		lastFlush_ = now;
		startOfPeriod_ = start;
		//file_->reset(new File(filename));
		//close();
		file_ = new File(filename);
		//printf("new File is born\n");
	}
}

std::string LogFile::getLogFileName(const std::string& basename,time_t* now) {
	std::string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	char pidbuf[32];
	struct tm tm;
	*now = time(NULL);
	gmtime_r(now,&tm);
	strftime(timebuf,sizeof(timebuf),".%Y%m%d-%H%M%S",&tm);
	filename += timebuf;
	snprintf(pidbuf,sizeof(pidbuf),".%d",::getpid());
	filename += pidbuf;
	filename += ".log";

	return filename;
}
