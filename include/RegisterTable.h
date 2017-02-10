#ifndef MUDUO_BASE_REGISTERTABLE_H__
#define MUDUO_BASE_REGISTERTABLE_H__

//#include "noncopyable.h"
//#include "Callbacks.h"
#include <map>
#include <string>

namespace muduo {
class RegisterTable {
public:
//	void init();
	void myRegister(std::string& conn);
	void myUnregister(std::string& conn);
	int getValue(std::string& conn);
	void addValue(std::string& conn);
//	void destroy();
private:
	static int count_;
	std::map<std::string,int> forId_;
	std::map<int,int> choose_;
};
	
//	int count_ = 0;
}

#endif // MUDUO_BASE_REGISTERTABLE_H__
