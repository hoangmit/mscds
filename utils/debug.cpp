#include "debug.h"
#include <iostream>

using namespace std;

namespace utils {

bool DbgHelper::flag = false;
uint64_t DbgHelper::counter_ = 0;

}//namespace