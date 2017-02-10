#ifndef MUDUO_BASE_PROCESSINFO_H__
#define MUDUO_BASE_PROCESSINFO_H__

#include "Timestamp.h"
#include <vector>
#include <string>

namespace muduo {
namespace ProcessInfo {
	pid_t pid();
	std::string pidString();
	uid_t uid();
	std::string username();
	uid_t euid();
	Timestamp startTime();

	std::string hostname();

	// read /proc/self/status
	std::string procStatus();
	
	int openedFiles();
	int maxOpenFiles();

	int numThreads();
	std::vector<pid_t> threads();
}
}

#endif // MUDUO_BASE_PROCESSINFO_H__
