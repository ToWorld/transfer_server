#include <string>
#include "RegisterTable.h"
//#include "TcpConnection.h"
//#include "Callbacks.h"

using namespace muduo;

int RegisterTable::count_ = 0;

/*
void RegisterTable::init() {
	;
}*/

void RegisterTable::myRegister(std::string& conn) {
	// ....
	if (forId_.find(conn)==forId_.end() || choose_.find(forId_[conn])==choose_.end()) {
		printf("%s starts to register\n",conn.c_str());
		forId_.insert(std::pair<std::string,int>(conn,count_));
		choose_.insert(std::pair<int,int>(count_,1));
		count_++;
	}
}

void RegisterTable::myUnregister(std::string& conn) {
	if (forId_.find(conn)!=forId_.end()&&choose_.find(forId_[conn])!=choose_.end()) {
		auto itr1 = forId_.find(conn);
		int id1 = forId_[conn];
		auto itr2 = choose_.find(id1);
		forId_.erase(itr1);
		choose_.erase(itr2);
	}
}

int RegisterTable::getValue(std::string& conn) {
	if (forId_.find(conn)!=forId_.end()&&choose_.find(forId_[conn])!=choose_.end()) {
		int result =  choose_[forId_[conn]];
		printf("result == %d\n",result);
		return result;
	}
	return 0;
}

void RegisterTable::addValue(std::string& conn) {
	if (forId_.find(conn)!=forId_.end()&&choose_.find(forId_[conn])!=choose_.end()) {
		choose_[forId_[conn]] += 1;
	}
}
