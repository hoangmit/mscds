#include "remote_file_impl.h"
#include "utils/file_utils.h"

#include <algorithm>

namespace mscds {


void FileCache::load_files(const std::string &prefix) {
	bitmap.load(prefix + ".meta_info");
	FileCache::MetaInfo minfo;
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
	open_ = true;
	n_blocks = (info.filesize + blocksize - 1) / blocksize;
	filesize_ = info.filesize;
}

void FileCache::create_files(const std::string &prefix) {
	MetaInfo minfo;
	minfo.filesize = this->info.filesize;
	minfo.update_time = this->info.last_update;
	minfo.blocksize = this->blocksize;
	//TODO: compute url hash
	minfo.urlhash = 0;
	bitmap.create(prefix + ".meta_info", (info.filesize + blocksize - 1) / blocksize,
		sizeof(MetaInfo), (char*)&minfo);
	datafl.create_rw(prefix + ".data", info.filesize);
	open_ = true;
	n_blocks = (info.filesize + blocksize - 1) / blocksize;
	filesize_ = info.filesize;
}

void FileCache::close() {
	bitmap.close();
	datafl.close();
	open_ = false;
}

void FileCache::remove_files(const std::string &suffix) {
	std::remove((suffix + ".meta_info").c_str());
	std::remove((suffix + ".data").c_str());
}

//-----------------------------------------------------------------------------

FilecacheRemoteFile::FilecacheRemoteFile(const std::string &url, const std::string &prefix, bool refresh_data):
		df(fc, url) {
	fc.info.filesize = 0; curpos = 0; fc.blocksize = fc.default_block_size;
	RemoteFileInfo remote_info;
	df.getInfo(remote_info);
	std::string metafile = prefix;
	if (!refresh_data && utils::file_exists(metafile)) {
		fc.load_files(prefix);
		if (fc.info != remote_info)
			throw remoteio_error("remote file information mismatched");
	}
	else {
		fc.info = remote_info;
		fc.create_files(metafile);
	}
	curpos = 0;
	fc.hit_count_ = 0;
	fc.total_count_ = 0;
}

/*char *FilecacheRemoteFile::create_map(size_t start, size_t len) {
	assert(start + len < info.filesize);
	return fetch(start, len);
}*/

void FilecacheRemoteFile::inspect(const std::string &param, std::ostream &out) const {
	out << "total_page_request = " << fc.total_count_ << std::endl;
	out << "hit_count = " << fc.hit_count_ << std::endl;
}

size_t FilecacheRemoteFile::read(char *dest, size_t size) {
	if (curpos + size > fc.info.filesize)
		size = curpos + size - fc.info.filesize;
	char* ptr = df.fetch(curpos, size);
	memcpy(dest, ptr, size);
	curpos += size;
	return size;
}

char FilecacheRemoteFile::peek() {
	if (curpos >= fc.info.filesize) return 0;
	char* ptr = df.fetch(curpos, 1);
	return *ptr;
}

//-----------------------------------------------------------------------------

char* SimpleDataFetcher::fetch(size_t start, size_t len) {
	if (len == 0) return (char*)fc.start_ptr() + start;
	assert(start + len <= fc.info.filesize);

	size_t end = ((start + len - 1) / fc.blocksize) + 1;
	size_t p = start / fc.blocksize;

	while (p < end) {
		if (!fc.check_blk(p)) {
			size_t rqsz = std::min<size_t>(fc.blocksize, fc.info.filesize - p * fc.blocksize);
			hobj.read_cont(p * fc.blocksize, rqsz, fc.ptr_blk(p));
			fc.set_blk(p);
		}
		else {
			fc.hit_count_++;
		}
		fc.total_count_++;
		p += 1;
	}
	return (char*)fc.start_ptr() + start;
}

//-------------------------------------------------------------------------

void ParallelDataFetcher::RangeDataRequest::init(FileCache *fc_, std::mutex *mt_, size_t start, size_t end_comp, size_t end_opt) {
	fc = fc_;
	mt = mt_;
	this->blocksize = fc->blocksize;

	i_blk = start;
	opt_end_blk = end_opt;
	complete_blk = end_comp;

	last_blk = fc->count_blk() - 1;
	lastblksize = fc->filesize() % fc->blocksize;
	if (lastblksize == 0) lastblksize = fc->blocksize;
	if (start >= end_comp)
		done_job.set_value();
}

void ParallelDataFetcher::RangeDataRequest::receive(const char *data, size_t len) {
	size_t buffsize = buff.size();
	if (buffsize + len >= blocksize) {
		std::lock_guard<std::mutex> lg(*mt);
		size_t rem = blocksize - buffsize;
		if (!check_blk(i_blk)) {
			char * ptr = fc->ptr_blk(i_blk);
			std::memcpy(ptr, buff.c_str(), buffsize);
			std::memcpy(ptr + buffsize, data, rem);
			set_blk(i_blk);
		}
		else fc->skip_count_++;
		advance();
		data += rem; len -= rem;
		buff.clear(); buffsize = 0;
		while (len >= blocksize) {
			if (!check_blk(i_blk)) {
				std::memcpy(fc->ptr_blk(i_blk), data, blocksize);
				set_blk(i_blk);
			}
			else fc->skip_count_++;
			advance();
			data += blocksize; len -= blocksize;
		}
	}
	assert(buffsize + len < blocksize);
	if (len > 0)
		buff.append(data, len);
	if (i_blk == last_blk && buff.size() == lastblksize) {
		std::lock_guard<std::mutex> lg(*mt);
		if (!check_blk(i_blk)) {
			std::memcpy(fc->ptr_blk(i_blk), buff.c_str(), buff.size());
			set_blk(i_blk);
		} else fc->skip_count_++;
		advance();
		buff.clear();
	}
}

std::vector<std::pair<size_t, size_t> > ParallelDataFetcher::scan(size_t stb, size_t edb) {
	std::vector<std::pair<size_t, size_t> > dl_intv;
	size_t p = stb;
	while (p < edb) {
		while (p < edb && (check_blk(p))) ++p;
		size_t r_start = p, last_m = p;
		size_t gap = 0;
		while (p < edb) {
			if (!check_blk(p)) {
				last_m = p;
				gap = 0;
			}
			else {
				++gap;
				if (gap > max_gap_size) break;
			}
			++p;
		}
		size_t r_end = last_m + 1;
		if (r_start < p)
			dl_intv.push_back(std::make_pair(r_start, r_end));
		++p;
	}
	return dl_intv;
}

}//namespace
