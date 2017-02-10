#include "AsyncLogging.h"
#include "LogStream.h"
#include "LogFile.h"
#include "Logging.h"

#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"

#include <unistd.h>
#include <vector>
#include <functional>
#include <memory>

using namespace muduo;

void AsyncLogging::append(const char* logline,int len) {
	//printf("into AsyncLogging::append\n");
	//printf("into AsyncLogging::append: %s\n",logline);
	muduo::MutexLockGuard lock(mutex_);
//	printf("--------------------------------------------------------------\n");
//	printf("currentBuffer_->avail() == %d\n",currentBuffer_->avail());
//	printf("len == %d\n",len);
	if (currentBuffer_->avail() > len) {
		// most common case: buffer is not full, copy data here
		currentBuffer_->append(logline,len);
	}
	else {	// buffer is full, push it, and find next buffer
		buffers_.push_back(std::move(currentBuffer_));
//		currentBuffer_ = NULL;
		if (nextBuffer_) {	// is there is one already, use it
			currentBuffer_ = std::move(nextBuffer_);
		}
		else {
			currentBuffer_ = new Buffer;	// Rarely happens
		}
		currentBuffer_->append(logline,len);
		cond_.notify();
	}
}

void AsyncLogging::threadFunc() {
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_,rollSize_,true);
	//printf("basename_------------------=%s\n",basename_.c_str());
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	/*
	if (newBuffer1 == NULL) {
		printf("newBuffer1 is NULL");	
	}
	if (newBuffer2 == NULL) {
		printf("newBuffer2 is NULL");	
	}
	*/
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);
	while (running_) {
		//printf("into running_\n");
		assert(newBuffer1!=NULL && (newBuffer1->length() == 0));
		assert(newBuffer2!=NULL && (newBuffer2->length() == 0));
		assert(buffersToWrite.empty());

		{
			
			muduo::MutexLockGuard lock(mutex_);
			if (buffers_.empty()) {
				cond_.waitForSeconds(flushInterval_);
			}
			//printf("currentBuffer_ = %s\n",currentBuffer_->data());
			buffers_.push_back(std::move(currentBuffer_));
//			delete currentBuffer_;
//			currentBuffer_ = NULL;	// memory leak
			currentBuffer_ = std::move(newBuffer1);
//			newBuffer1->reset();
//			delete newBuffer1;
			newBuffer1 = NULL;
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_) {	
				nextBuffer_ = std::move(newBuffer2);
//				newBuffer2->reset();
//				delete newBuffer2;
				newBuffer2 = NULL;
			}
		}
		assert(!buffersToWrite.empty());
		/*
		if (buffersToWrite.size() > 25) {
			const char* dropMsg = "Dropped log message\n";
			fprintf(stderr,"%s",dropMsg);
			output.append(dropMsg,strlen(dropMsg));
			buffersToWrite.erase(buffersToWrite.begin(),buffersToWrite.end()-2);
		}*/
			
		for (size_t i = 0; i < buffersToWrite.size(); ++i) {
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?
//			printf("buffersToWrite[%d]->data() == %s\n",i,buffersToWrite[i]->data());
			output.append(buffersToWrite[i]->data(),buffersToWrite[i]->length());
		}
		//printf("buffersToWrite.size() == %d\n",buffersToWrite.size());
		if (buffersToWrite.size() > 2) {
			// drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);
		}
		
		if (!newBuffer1) {
			//printf("newBuffer1 is NULL\n");
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer1->reset();
		}
		if (!newBuffer2) {
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer2->reset();
		}
		buffersToWrite.clear();
		output.flush();
	}
	output.flush();
}


