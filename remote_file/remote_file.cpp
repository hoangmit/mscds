
#include "remote_file.h"
#include "remote_file_impl.h"
#include "utils/md5.h"

#include "utils/file_utils.h"

#include <unordered_map>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <cstring>

using namespace std;


namespace mscds {


//---------------------------------------------------------

RemoteFileRepository::RemoteFileRepository(): cachemem(nullptr) {
	_cache_dir = default_repository();
}

RemoteFileRepository::~RemoteFileRepository() {
	if (cachemem != nullptr) delete[] cachemem;
}

RemoteFileHdl RemoteFileRepository::open(const std::string &url, bool refresh_data, RemoteCacheType cachetype) {
	if (cachetype == NOCACHE) {
		return std::make_shared<NocacheRemoteFile>(url);
	}else
		if (cachetype == PRIVATE_MEM_CACHE) {
			return std::make_shared<PrivateMemcacheRemoteFile>(url);
		}else
			if (cachetype == FILE_CACHE) {
		std::string path = _cache_dir + utils::MD5::hex(url);
		std::shared_ptr<FilecacheRemoteFile> h = std::make_shared<FilecacheRemoteFile>(url, path, refresh_data);

		return h;
	}
	else { throw runtime_error("unknown type"); }
}

std::string RemoteFileRepository::default_repository() {
	return utils::get_temp_path();
}

void RemoteFileRepository::change_cache_dir(const std::string& dir) {
	if (dir != "")
		_cache_dir = dir;
	else
		_cache_dir = default_repository();
}



}//namespace

