
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
#include <cstring>

using namespace std;


namespace mscds {

struct NocacheRemoteFile : RemoteFileInt {
	NocacheRemoteFile() { info.filesize = 0; curpos = 0; }
	NocacheRemoteFile(const std::string& url) : _url(url), hobj(url) { info.filesize = 0; curpos = 0; }

	std::string url() { return _url; }
	size_t size() { return info.filesize; }
	time_t update_time() { return info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_once(curpos, size, dest);
		curpos += size;
		return size;
	}

	char peek() {
		if (curpos >= info.filesize) return 0;
		char ch;
		hobj.read_once(curpos, 1, &ch);
		return ch;
	}

	void seekg(size_t pos) { curpos = pos; }
	size_t tellg() const { return curpos; }
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
	size_t read(char *dest, size_t size) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_once(curpos, size, dest);
		curpos += size;
		return size;
	}

	char peek() {
		if (curpos >= info.filesize) return 0;
		char ch;
		hobj.read_once(curpos, 1, &ch);
		return ch;
	}

	void seekg(size_t pos) { curpos = pos; }
	size_t tellg() const { return curpos; }
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
	FilecacheRemoteFile() { info.filesize = 0; curpos = 0; blocksize = default_block_size; }
	FilecacheRemoteFile(const std::string& url, const std::string& prefix, bool refresh_data = false);

	std::string url() { return _url; }
	size_t size() { return info.filesize; }
	time_t update_time() { return info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size);
	char peek();


	void seekg(size_t pos) { curpos = pos; }
	size_t tellg() const { return curpos; }
	bool eof() const { return curpos == info.filesize; }

	bool has_mapping() { return true; }
	size_t max_map_size() { return info.filesize; }
	char* create_map(size_t start, size_t len) {
		assert(start + len < info.filesize);
		return fetch(start, len);
	}
	virtual void release_map(char* ptr) {}
	const unsigned int default_block_size = 16 * 1024;
	size_t hit_count() const { return _hit_count; }
	size_t total_count() const { return _total_count; }
	void inspect(const std::string& param, std::ostream& out) const {
		out << "total_page_request = " << _total_count << std::endl;
		out << "hit_count = " << _hit_count << std::endl;
	}
private:
	void load_files(const std::string& prefix);
	void create_files(const std::string& prefix);
	static void remove_files(const std::string& prefix);
private:
	char* fetch(size_t start, unsigned int len);

public:
	RemoteFileInfo info;
	std::string _url;
	
	HttpFileObj hobj;
	mman::MemoryMappedFile datafl;
	ExternalBitMap bitmap;
	uint32_t blocksize;
	
	friend class RemoteFileRepository;
	size_t curpos;
	uint32_t _hit_count, _total_count;
private:
	struct MetaInfo {
		uint64_t urlhash;
		size_t filesize;
		time_t update_time;
		uint32_t blocksize;
	};
};

FilecacheRemoteFile::FilecacheRemoteFile(const std::string &url, const std::string &prefix, bool refresh_data) : _url(url), hobj(url) {
	info.filesize = 0; curpos = 0; blocksize = default_block_size;
	RemoteFileInfo remote_info;
	hobj.getInfo(remote_info);
	std::string metafile = prefix;
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
	_hit_count = 0;
	_total_count = 0;
}

size_t FilecacheRemoteFile::read(char *dest, size_t size) {
	if (curpos + size > info.filesize)
		size = curpos + size - info.filesize;
	char* ptr = fetch(curpos, size);
	memcpy(dest, ptr, size);
	curpos += size;
	return size;
}

char FilecacheRemoteFile::peek() {
	if (curpos >= info.filesize) return 0;
	char ch;
	char* ptr = fetch(curpos, 1);
	return *ptr;
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

char *FilecacheRemoteFile::fetch(size_t start, unsigned int len) {
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
		else {
			_hit_count++;
		}
		_total_count++;
		ptr += blocksize;
		nstart += blocksize;
		p += 1;
	}
	return (char*)datafl.addr + start;
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

void RemoteFileRepository::change_cache_dir(const std::string& dir) {
	if (dir != "")
		_cache_dir = dir;
	else
		_cache_dir = default_repository();
}



}//namespace

