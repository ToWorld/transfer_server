#include "ProcessInfo.h"
#include "FileUtil.h"
#include "StringPiece.h"
#include <algorithm>
#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

namespace muduo {
namespace detail {
__thread int t_numOpenedFiles = 0;
int fdDirFilter(const struct dirent* d) {
	if (::isdigit(d->d_name[0])) {
		t_numOpenedFiles++;
	}
	return 0;
}

__thread std::vector<pid_t>* t_pids = NULL;
int taskDirFilter(const struct dirent* d) {
	if (::isdigit(d->d_name[0])) {
		t_pids->push_back(atoi(d->d_name));
	}
	return 0;
}

int scanDir(const char* dirpath,int (*filter)(const struct dirent*)) {
	struct dirent** namelist = NULL;
	int result = ::scandir(dirpath,&namelist,filter,alphasort);
	assert(namelist == NULL);
	return result;
}

Timestamp g_startTime = Timestamp::now();
}
}

using namespace muduo;
using namespace muduo::detail;

pid_t ProcessInfo::pid() {
	return ::getpid();
}

std::string ProcessInfo::pidString() {
	char buf[32];
	snprintf(buf,sizeof(buf),"%d",pid());
	return buf;
}

uid_t ProcessInfo::uid() {
	return ::getuid();
}

std::string ProcessInfo::username() {
	struct passwd pwd;
	struct passwd* result = NULL;
	char buf[8192];
	const char* name = "unknownuser";

	getpwuid_r(uid(),&pwd,buf,sizeof(buf),&result);
	if (result) {
		name = pwd.pw_name;
	}
	return name;
}

uid_t ProcessInfo::euid() {
	return ::geteuid();
}

Timestamp ProcessInfo::startTime() {
	return g_startTime;
}

std::string ProcessInfo::hostname() {
	char buf[64] = "unknownhost";
	buf[sizeof(buf)-1] = '\0';
	::gethostname(buf,sizeof(buf));
	return buf;
}

std::string ProcessInfo::procStatus() {
	std::string result;
	FileUtil::readFile("/proc/self/status",65536,&result);

	return result;
}

int ProcessInfo::openedFiles() {
	t_numOpenedFiles = 0;
	scanDir("/proc/self/fd",fdDirFilter);
	return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles() {
	struct rlimit rl;
	if (::getrlimit(RLIMIT_NOFILE,&rl)) {
		return openedFiles();
	}
	else {
		return static_cast<int>(rl.rlim_cur);
	}
}

int ProcessInfo::numThreads() {
	int result = 0;
	std::string status = procStatus();
	size_t pos = status.find("Threads:");
	if (pos != std::string::npos) {
		result = ::atoi(status.c_str() + pos + 8);
	}
	return result;
}

std::vector<pid_t> ProcessInfo::threads() {
	std::vector<pid_t> result;
	t_pids = &result;
	scanDir("/proc/self/task",taskDirFilter);
	t_pids = NULL;
	std::sort(result.begin(),result.end());
	return result;
}
