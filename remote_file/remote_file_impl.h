#pragma once

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
		hit_count_(0), skip_count_(0), total_count_(0) { }
	/*size_t hit_count() const { return hit_count_; }
	size_t total_count() const { return total_count_; } */

	uint32_t hit_count_, skip_count_, total_count_;

	void load_files(const std::string& prefix);
	void create_files(const std::string& prefix);
	void close();
	static void remove_files(const std::string& prefix);

	bool is_open() const { return open_; }

	uint32_t blocksize;
	static const unsigned int default_block_size = 16 * 1024;
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

		std::string buff;
		typedef size_t blk_idx_t;

		size_t blocksize, lastblksize;
		blk_idx_t last_blk, complete_blk, opt_end_blk, i_blk;
		std::promise<void> done_job, done_optional;

		std::shared_ptr<HttpFileObj> hobj;

		void receive(const char* data, size_t len);
		

		void stop() { std::lock_guard<std::mutex> lg(*mt); hobj->stop_read(); }
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
	ParallelDataFetcher(FileCache& fc_, const std::string& url) : fc(fc_), url_(url), h1(url) { init(); }


	void init() { scan_ptx = 0; full_data = false; }

	std::mutex write_mt;

	char* fetch(size_t start, size_t len) {
		if (len == 0 || full_data) return (char*)fc.start_ptr() + start;
		//std::cout << "fetch: " << start << ", " << len << std::endl;
		assert(start + len <= fc.info.filesize);

		size_t end_blk = ((start + len - 1) / fc.blocksize) + 1;
		size_t start_blk = start / fc.blocksize;

		std::vector<std::pair<size_t, size_t> > vt;
		{
			std::lock_guard<std::mutex> lg(write_mt);
			vt = scan(start_blk, end_blk);
		}
		if (vt.size() > 1) {
			for (unsigned int i = 0; i < vt.size() - 1; ++i)
				dispatch_request(vt[i].first, vt[i].second, vt[i].second);
		}
		bool contt = false;
		if (vt.size() > 0) {
			auto & last = vt[vt.size() - 1];
			if (end_blk - last.second < max_gap_size) {
				dispatch_request(last.first, last.second, last.second + max_forward_size);
				contt = true;
			}
			else
				dispatch_request(last.first, last.second, last.second);
		}
		if (!contt) {
			//size_t x = find_first_free();
			//if (!full_data)
			//	dispatch_request(x, x, x + max_forward_size);
		}
		return (char*)fc.start_ptr() + start;
	}

	std::queue<std::shared_ptr<RangeDataRequest> > opt_jobs;
	void dispatch_request(size_t start, size_t end, size_t end_opt) {
		//std::cout << "dispatch " << start << ", " << end << ", " << end_opt << std::endl;
		while (opt_jobs.size() > 1)
			kill_one_job();
		std::shared_ptr<RangeDataRequest> rd = std::make_shared<RangeDataRequest>();
		rd->init(&fc, &write_mt, start, end, end_opt);
		rd->hobj.reset(new HttpFileObj(url()));
		std::future<void> cj = rd->done_job.get_future();
		size_t st = rd->fc->start_blk(start);
		size_t ed = rd->fc->start_blk(end_opt);
		//using namespace std::placeholders;
		std::async(std::launch::async, [rd, st, ed]() {
			rd->hobj->read_cont(st, ed - st, [rd](const char* ptr, size_t len){
				rd->receive(ptr, len);
			});
		});
		cj.wait();
		opt_jobs.push(rd);
	}

	bool is_optional_running() {
		return !opt_jobs.empty();
	}

	void kill_one_job() {
		auto j = opt_jobs.front();
		j->stop();
		opt_jobs.pop();
	}

	void cancel_optional() {
		while (!opt_jobs.empty()) {
			kill_one_job();
		}
	}

	size_t scan_ptx;
	size_t find_first_free() {
		std::lock_guard<std::mutex> lg(write_mt);
		while (scan_ptx < fc.count_blk() && fc.check_blk(scan_ptx)) ++scan_ptx;
		if (scan_ptx < fc.count_blk())
			return scan_ptx;
		else {
			full_data = true;
			return 0;
		}
	}

	bool check_blk(size_t p) const {
		return fc.check_blk(p);
	}

	//tested
	std::vector<std::pair<size_t, size_t> > scan(size_t stb, size_t edb);
	const uint32_t max_gap_size = 1024;
	const uint32_t max_forward_size = 1024;
	
	bool full_data;

	std::string url() const { return url_; }
	void getInfo(RemoteFileInfo& inf) { h1.getInfo(inf); }
private:
	HttpFileObj h1;
	std::string url_;
	FileCache& fc;
};


struct FilecacheRemoteFile : RemoteFileInt {
public:
	FilecacheRemoteFile() : df(fc) { fc.info.filesize = 0; curpos = 0; }
	FilecacheRemoteFile(const std::string& url, const std::string& prefix, bool refresh_data = false);

	std::string url() const { return df.url(); }
	size_t size() const { return fc.info.filesize; }
	time_t update_time() const { return fc.info.last_update; }

	void close()  {}
	size_t read(char *dest, size_t size);
	char peek();

	void seekg(size_t pos) { curpos = pos; }
	size_t tellg() const { return curpos; }
	bool eof() const { return curpos == fc.info.filesize; }

	//bool has_mapping() { return true; }
	//size_t max_map_size() { return info.filesize; }
	//char* create_map(size_t start, size_t len);
	//virtual void release_map(char* ptr) {}

	void inspect(const std::string& param, std::ostream& out) const;
private:

public:
	ParallelDataFetcher df;
	//SimpleDataFetcher df;
	FileCache fc;
	friend class RemoteFileRepository;
	size_t curpos;
};

}//namespace
