#pragma once

/**

Private implement Remote file access

(current only HTTP/HTTPS file is supported.)

*/

#include "remote_file.h"
#include "http_client.h"
#include "ext_bitmap.h"
#include "memmapfile.h"

#include <stdint.h>
#include <ctime>
#include <string>
#include <list>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <queue>


namespace mscds {

struct NocacheRemoteFile : RemoteFileInt {
	NocacheRemoteFile() { info.filesize = 0; curpos = 0; }
	NocacheRemoteFile(const std::string& url) : _url(url), hobj(url) { info.filesize = 0; curpos = 0; }

	std::string url() const { return _url; }
	size_t size() const { return info.filesize; }
	time_t update_time() const { return info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_cont(curpos, size, dest);
		curpos += size;
		return size;
	}

	char peek() {
		if (curpos >= info.filesize) return 0;
		char ch;
		hobj.read_cont(curpos, 1, &ch);
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

	std::string url() const { return _url; }
	size_t size() const { return info.filesize; }
	time_t update_time() const { return info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size) {
		if (curpos + size > info.filesize)
			size = curpos + size - info.filesize;
		hobj.read_cont(curpos, size, dest);
		curpos += size;
		return size;
	}

	char peek() {
		if (curpos >= info.filesize) return 0;
		char ch;
		hobj.read_cont(curpos, 1, &ch);
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


struct FileCache {
	FileCache() : open_(false), blocksize(default_block_size),
		n_blocks(0), filesize_(0),
		hit_count_(0), skip_count_(0), total_count_(0), req_count_(0) {}
	/*size_t hit_count() const { return hit_count_; }
	size_t total_count() const { return total_count_; } */

	uint32_t hit_count_, skip_count_, total_count_, req_count_;

	void load_files(const std::string& prefix);
	void create_files(const std::string& prefix);
	void close();
	static void remove_files(const std::string& prefix);

	bool is_open() const { return open_; }

	uint32_t blocksize;
	static const unsigned int default_block_size = 8 * 4 * 16 * 1024;
	RemoteFileInfo info;
	bool check_blk(size_t p) const { return bitmap.getbit(p); }
	void set_blk(size_t p) { bitmap.setbit(p); }
	void clear_blk(size_t p) { bitmap.clearbit(p); }

	size_t count_blk() const { return n_blocks; }
	size_t filesize() const { return filesize_; }

	inline char* ptr_blk(size_t p) { return ((char*)datafl.addr) + p * blocksize; }
	size_t start_blk(size_t p) const { if (p < n_blocks) return p * blocksize; else return filesize_; }
	size_t end_blk(size_t p) const { if (p < n_blocks) return (p + 1) * blocksize; else return filesize_; }
	inline char* start_ptr() { return ((char*)datafl.addr); }
private:
	mman::MemoryMappedFile datafl;
	ExternalBitMap bitmap;

	size_t filesize_, n_blocks;

	bool open_;
	struct MetaInfo {
		uint64_t urlhash;
		size_t filesize;
		time_t update_time;
		uint32_t blocksize;
	};
};

struct SimpleDataFetcher {
	SimpleDataFetcher(FileCache& fc_) : fc(fc_) {}
	SimpleDataFetcher(FileCache& fc_, const std::string& url) : fc(fc_), url_(url), hobj(url) {}

	char* fetch(size_t start, size_t len);
	std::string url() const { return url_; }
	void getInfo(RemoteFileInfo& inf) { hobj.getInfo(inf); }
private:
	HttpFileObj hobj;
	std::string url_;
	FileCache& fc;
};

}//namespace


#include <thread>
#include <mutex>
#include <cstring>
#include <vector>
#include <utility>
#include <stdint.h>
#include <future>
#include <deque>

namespace mscds {



struct ParallelDataFetcher {

	~ParallelDataFetcher() {
		cancel_optional();
	}

	struct RangeDataRequest {
		RangeDataRequest() : fc(nullptr), mt(nullptr) {}
		~RangeDataRequest() {
			if (fc && mt) stop();
		}
		void init(FileCache* fc_, std::mutex* mt_,
			size_t start, size_t end_comp, size_t end_opt);
		FileCache* fc;
		std::mutex* mt;
		void receive(const char* data, size_t len);
		void stop();
		std::promise<void> done_job, done_optional;
		std::shared_ptr<HttpFileObj> hobj;
	private:
		std::string buff;
		typedef size_t blk_idx_t;

		size_t blocksize, lastblksize;
		blk_idx_t last_blk, complete_blk, opt_end_blk, i_blk;

	private:
		inline void advance() {
			++i_blk;
			++fc->total_count_;
			//std::cout << ".";
			if (i_blk == complete_blk)
				done_job.set_value();
			if (i_blk == opt_end_blk) {
				done_optional.set_value();
			} else {
				if (i_blk > opt_end_blk)
					throw "overflow";
			}
		}
		inline bool check_blk(size_t blk) const {
			if (blk >= opt_end_blk) return false;
			else return fc->check_blk(blk);
		}
		inline void set_blk(size_t blk) const {
			fc->set_blk(blk);
		}
	};


	ParallelDataFetcher(FileCache& fc_) : fc(fc_) { init(); }
	ParallelDataFetcher(FileCache& fc_, const std::string& url) : fc(fc_), url_(url) { init(); }

	char* fetch(size_t start, size_t len);
	std::string url() const { return url_; }
	void getInfo(RemoteFileInfo& inf) { HttpFileObj h1(url_); h1.getInfo(inf); }

private:
	void init() { scan_ptx = 0; full_data = false; }

	std::mutex write_mt;
	std::queue<std::shared_ptr<RangeDataRequest> > opt_jobs;
	void dispatch_request(size_t start, size_t end, size_t end_opt);

	bool is_optional_running();
	void kill_one_job();
	void cancel_optional();

	size_t scan_ptx;
	bool full_data;
	size_t find_first_free();

	inline bool check_blk(size_t p) const {
		return fc.check_blk(p);
	}

	std::vector<std::pair<size_t, size_t> > scan(size_t stb, size_t edb);
	static const uint32_t max_gap_size = 1024;
	static const uint32_t max_forward_size = 1024;	

private:
	//HttpFileObj h1;
	std::string url_;
	FileCache& fc;
};


struct FilecacheRemoteFile : RemoteFileInt {
public:
	FilecacheRemoteFile() : df(fc) { fc.info.filesize = 0; curpos = 0; }
	FilecacheRemoteFile(const std::string& url, const std::string& prefix, bool refresh_data = false);
	~FilecacheRemoteFile() {
		//std::cout << "destruction" << std::endl;
	}

	std::string url() const { return df.url(); }
	size_t size() const { return fc.info.filesize; }
	time_t update_time() const { return fc.info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size);
	char peek();

	void seekg(size_t pos) { curpos = pos; }
	size_t tellg() const { return curpos; }
	bool eof() const { return curpos == fc.info.filesize; }

	bool has_mapping() { return true; }
	size_t max_map_size() { return fc.filesize(); }
	char* create_map(size_t start, size_t len) { return df.fetch(start, len); }
	void release_map(char* ptr) {}

	void inspect(const std::string& param, std::ostream& out) const;
private:
	FileCache fc;
	//ParallelDataFetcher df;
	SimpleDataFetcher df;
	
	friend class RemoteFileRepository;
	size_t curpos;
};

}//namespace
