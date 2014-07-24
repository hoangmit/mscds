#pragma once

/** 

Implement Remote file access

(current only HTTP/HTTPS file is supported.)

*/

#include <string>
#include <memory>
#include <stdexcept>

#include "error.h"

namespace mscds {

class RemoteFileInfoInt {
public:
	virtual std::string url() const = 0;
	virtual size_t size() const = 0;
	virtual time_t update_time() const = 0;
};

class RemoteFileInt : public RemoteFileInfoInt {
public:
	virtual void close() = 0;
	virtual size_t read(char* dest, size_t size) = 0;
	virtual void seekg(size_t pos) = 0;
	virtual size_t tellg() const = 0;
	virtual char peek() = 0;
	virtual bool eof() const = 0;
	virtual ~RemoteFileInt() {}

	virtual size_t hit_count() const { return 0; }
	virtual size_t total_count() const { return 0; }
	virtual void inspect(const std::string& param, std::ostream& out) const {}
};

typedef std::shared_ptr<RemoteFileInt> RemoteFileHdl;

class RemoteFileRepository {
public:
	enum RemoteCacheType {NOCACHE, PRIVATE_MEM_CACHE, FILE_CACHE}; //SHARED_MEM_CACHE

	RemoteFileRepository();
	~RemoteFileRepository();
	RemoteFileRepository(const std::string& dir);

	void change_cache_dir(const std::string& dir);
	
	RemoteFileHdl open(const std::string& url, bool refresh_data = false,
		RemoteCacheType cachetype = FILE_CACHE);
	
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
