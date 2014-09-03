
#pragma once

/* \file

Implement HTTP client to read a remote file.
*/

#include <string>
#include <memory>
#include <functional>

namespace mscds {

struct RemoteFileInfo {
	size_t filesize;
	time_t last_update;
	bool operator==(const RemoteFileInfo& other) const {
		return filesize == other.filesize && last_update == other.last_update;
	}
	bool operator!=(const RemoteFileInfo& other) const { return !(*this == other); }
};

/// HTTP file
struct HttpFileObj {
public:
	typedef std::function<void(const char*, size_t)> CallBack;

	HttpFileObj();
	~HttpFileObj();
	HttpFileObj(const std::string& url);
	void getInfo(RemoteFileInfo& info);
	void read_cont(size_t start, size_t len, char* dest);
	void read_cont(size_t start, size_t len, CallBack fx);
	void stop_read();
private:
	//unsigned int id_;
	void* pimpl;
	std::shared_ptr<void> pimpl_ctl_;
};

}