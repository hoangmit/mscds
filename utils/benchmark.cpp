#include "benchmark.h"

#include <iostream>
#include <string>
using namespace std;

namespace tests {

BenchmarkRegister* BenchmarkRegister::_inst = NULL;

BenchmarkRegister *BenchmarkRegister::getInst() {
	if (_inst == NULL)
		_inst = new BenchmarkRegister();
	return _inst;
}

void* BenchmarkRegister::add(const std::string &name, BenchmarkRegister::VoidFunc func) {
	_list.emplace_back(name, func);
	return this;
}

/*
const std::list<BenchmarkRegister::VoidFunc> &BenchmarkRegister::getList() const {
	return _list;
}*/

void BenchmarkRegister::run_all_bm()  {
	BenchmarkRegister * reg = getInst();
	cout << "Running all registered functions" << endl;
	for (auto& p :reg-> _list) {
		cout << "> " << p.first << endl;
		try {
			p.second();
		} catch (std::runtime_error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl;
		}
		cout << endl;
	}
}


}//namespace
