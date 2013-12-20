
#include "remote_file.h"
#include "strptime.h"

#include "http_client.h"
#include "ext_bitmap.h"
#include "memmapfile.h"
#include "utils/file_utils.h"

#include <unordered_map>
#include <ctime>
#include <cassert>
#include <algorithm>

using namespace std;


namespace mscds {

struct NocacheRemoteFile : RemoteFileInt {
	NocacheRemoteFile() { info.filesize = 0; curpos = 0; }
	NocacheRemoteFile(const std::string& url) : _url(url), hobj(url) { info.filesize = 0; curpos = 0; }

	std::string url() { return _url; }
	size_t size() { return info.filesize; }
	time_t update_time() { return info.last_update; }

	void close()  {}
	size_t read(size_t size, char *dest) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_once(curpos, size, dest);
		curpos += size;
		return size;
	}

	void seek(size_t pos) { curpos = pos; }
	size_t tell() const { return curpos; }
	bool eof() const { return curpos == info.filesize; }

	RemoteFileInfo info;
	size_t curpos;
	std::string _url;
	HttpFileObj hobj;
	friend class RemoteFileRepository;

};

//------------------------------------------------------------------------------------------------

struct PrivateMemcacheRemoteFile : RemoteFileInt {
	PrivateMemcacheRemoteFile() { info.filesize = 0; curpos = 0; }
	PrivateMemcacheRemoteFile(const std::string& url) : _url(url), hobj(url) { info.filesize = 0; curpos = 0; }

	std::string url() { return _url; }
	size_t size() { return info.filesize; }
	time_t update_time() { return info.last_update; }

	void close()  {}
	size_t read(size_t size, char *dest) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_once(curpos, size, dest);
		//get_http_file_data(url(), curpos, size, dest);
		curpos += size;
		return size;
	}

	void seek(size_t pos) { curpos = pos; }
	size_t tell() const { return curpos; }
	bool eof() const { return curpos == info.filesize; }
public:
	RemoteFileInfo info;
	size_t curpos;
	std::string _url;
	HttpFileObj hobj;

	unsigned int blocksize;
	size_t cacheblocks;

	char* memcache;
	friend class RemoteFileRepository;
private:
};

//------------------------------------------------------------------------------------------------

struct FilecacheRemoteFile : RemoteFileInt {
public:
	FilecacheRemoteFile() { info.filesize = 0; curpos = 0; blocksize = 8 * 1024; }
	FilecacheRemoteFile(const std::string& url, const std::string& prefix, bool refresh_data = false) : _url(url), hobj(url) {
		info.filesize = 0; curpos = 0; blocksize = 8 * 1024;

		RemoteFileInfo remote_info;
		hobj.getInfo(remote_info);
		std::string metafile = prefix + ".meta_info";
		if (!refresh_data && utils::file_exists(metafile)) {
			load_files(prefix);
			if (info != remote_info)
				throw remoteio_error("remote file information mismatched");
		}
		else {
			info = remote_info;
			create_files(metafile);
		}
		curpos = 0;
	}

	std::string url() { return _url; }
	size_t size() { return info.filesize; }
	time_t update_time() { return info.last_update; }

	void close()  {}
	size_t read(size_t size, char *dest);

	void seek(size_t pos) { curpos = pos; }
	size_t tell() const { return curpos; }
	bool eof() const { return curpos == info.filesize; }
private:
	void load_files(const std::string& prefix);
	void create_files(const std::string& prefix);
	static void remove_files(const std::string& prefix);
private:
	char* fetch(size_t start, unsigned int len) {
		assert(start + len <= info.filesize);
		size_t nstart = start - start % blocksize;
		size_t end = start + len;
		unsigned int p = nstart / blocksize;
		char* ptr = (char*)datafl.addr + nstart;
		while (nstart < end) {
			if (!bitmap.getbit(p)) {
				size_t rqsz = std::min<size_t>(blocksize, info.filesize - nstart);
				hobj.read_cont(nstart, rqsz, ptr);
				//get_http_file_data(_url, nstart, rqsz, ptr);
				bitmap.setbit(p);
			}
			ptr += blocksize;
			nstart += blocksize;
		}
		return (char*)datafl.addr + start;
	}

public:
	RemoteFileInfo info;
	std::string _url;
	
	HttpFileObj hobj;
	mman::MemoryMappedFile datafl;
	ExternalBitMap bitmap;
	uint32_t blocksize;
	
	friend class RemoteFileRepository;
	size_t curpos;
private:
	struct MetaInfo {
		uint64_t urlhash;
		size_t filesize;
		time_t update_time;
		uint32_t blocksize;
	};
};

size_t FilecacheRemoteFile::read(size_t size, char *dest) {
	if (curpos + size > info.filesize)
		size = curpos + size - info.filesize;
	char* ptr = fetch(curpos, size);
	memcpy(dest, ptr, size);
	curpos += size;
	return size;
}

void FilecacheRemoteFile::load_files(const std::string &prefix) {
	bitmap.load(prefix + ".meta_info");
	FilecacheRemoteFile::MetaInfo minfo;
	if (bitmap.get_ext_size() != sizeof(MetaInfo))
		throw remoteio_error("wrong ext info size");
	memcpy(&minfo, bitmap.get_extinfo(), sizeof(MetaInfo));
	this->info.filesize = minfo.filesize;
	this->info.last_update = minfo.update_time;
	this->blocksize = minfo.blocksize;
	//TODO: check url hash
	datafl.load_rw(prefix + ".data");
	if (datafl.len != info.filesize)
		throw remoteio_error("file size mismatched");
}

void FilecacheRemoteFile::create_files(const std::string &prefix) {
	MetaInfo minfo;
	minfo.filesize = this->info.filesize;
	minfo.update_time = this->info.last_update;
	minfo.blocksize = this->blocksize;
	//TODO: compute url hash
	minfo.urlhash = 0;
	bitmap.create(prefix + ".meta_info", (info.filesize + blocksize - 1) / blocksize, sizeof(MetaInfo), (char*)&minfo);
	datafl.create_rw(prefix + ".data", info.filesize);
}

void FilecacheRemoteFile::remove_files(const std::string &suffix) {
	std::remove((suffix + ".meta_info").c_str());
	std::remove((suffix + ".data").c_str());
}

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
		std::string path = _cache_dir + uri_encode(url);
		std::shared_ptr<FilecacheRemoteFile> h = std::make_shared<FilecacheRemoteFile>(url, path, refresh_data);

		return h;
	}
	else { throw runtime_error("unknown type"); }
}

std::string RemoteFileRepository::default_repository() {
	return utils::get_temp_path();
}



}//namespace

