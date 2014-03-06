
#pragma once

#include "remote_file_impl.h"
#include <string>
#include <memory>

struct HttpFileObj {
public:
	HttpFileObj();
	HttpFileObj(const std::string& url);
	void getInfo(RemoteFileInfo& info);
	void read_once(size_t start, size_t len, char* dest);
	void read_cont(size_t start, size_t len, char* dest);
private:
	void* impl;
	std::shared_ptr<void> _impl_ctl;
};

