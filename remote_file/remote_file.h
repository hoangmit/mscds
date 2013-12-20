#pragma once

#include <string>
#include <memory>
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

class RemoteFileInfoInt {
public:
	virtual std::string url() = 0;
	virtual size_t size() = 0;
	virtual time_t update_time() = 0;
};

class RemoteFileInt : public RemoteFileInfoInt {
public:
	virtual void close() = 0;
	virtual size_t read(size_t size, char* dest) = 0;
	virtual void seek(size_t pos) = 0;
	virtual size_t tell() const = 0;
	virtual bool eof() const = 0;
};

typedef std::shared_ptr<RemoteFileInt> RemoteFileHdl;

class RemoteFileRepository {
public:
	enum RemoteCacheType {NOCACHE, PRIVATE_MEM_CACHE, FILE_CACHE}; //SHARED_MEM_CACHE

	RemoteFileRepository();
	~RemoteFileRepository();
	RemoteFileRepository(const std::string& dir);
	
	RemoteFileHdl open(const std::string& url, bool refresh_data = false, RemoteCacheType cachetype = FILE_CACHE);
	
	size_t clean(unsigned int max_age);
	std::string cache_dir();
	
	/*
	unsigned int get_timeout();
	void set_timeout(unsigned int time_seconds);
	*/

	static std::string default_repository();
private:
	std::string _cache_dir;
	char * cachemem;
};


}//namespace
