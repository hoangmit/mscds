#pragma once

/** 
defines io error
*/

#include <string>
#include <stdexcept>

namespace mscds {

class remoteio_error : public ::std::exception {
public:
	remoteio_error() {}
	~remoteio_error() throw() {}
	remoteio_error(const remoteio_error& other) : msg(other.msg) {}
	remoteio_error(const std::string& _msg) : msg(_msg) {}
	const char* what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

}