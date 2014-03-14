#include "memmapfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#ifdef WIN32
#define PAGE_READONLY          0x02     
#define SECTION_MAP_READ    0x0004
#define FILE_MAP_READ       SECTION_MAP_READ

#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif


namespace mman {

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif


#ifdef WIN32
void MemoryMappedFile::load_r(const char *fname) {
	if (is_open()) throw std::runtime_error("still open");
	void *base;
	HANDLE fd,h;
	size_t len;
	fd = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (fd==INVALID_HANDLE_VALUE) {
		throw std::runtime_error("createfile");
	}	
	//len = GetFileSize(fd,0) - 1;len++;
	len = GetFileSize(fd,NULL);
	h = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL);
	if (h==NULL) {
		throw std::runtime_error("createfilemapping");
	}
	base = MapViewOfFile(h, FILE_MAP_READ, 0, 0, 0);
	if (base==NULL) {
		throw std::runtime_error("mapviewoffile");
	}
	this->mod = 1;
	this->addr = base;
	this->len = len;
	this->h1 = fd;
	this->h2 = h;
}

void MemoryMappedFile::close() {
	if (this->mod != 0) {		
		UnmapViewOfFile(addr);
		CloseHandle(h2);
		CloseHandle(h1);
		this->addr = NULL;
		this->mod = 0;
	}
}

void MemoryMappedFile::create_rw(const char *fname, uint64_t len) {
	if (is_open()) throw std::runtime_error("still open");
	void *base;
	HANDLE fd,h;
	fd = CreateFile(fname,(GENERIC_READ|GENERIC_WRITE),FILE_SHARE_READ,NULL,CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (fd==INVALID_HANDLE_VALUE) {
		throw std::runtime_error("createfile");
	}
	this->h1 = fd;
	LARGE_INTEGER li;
	li.QuadPart = len;
	if (!SetFilePointerEx(fd, li, NULL, FILE_BEGIN)) {throw std::runtime_error("cannot set pointer ");}
	if (!SetEndOfFile(fd)) {throw std::runtime_error("cannot set end of file");}
	h = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (h==NULL) {throw std::runtime_error("createfilemapping");}	
	this->h2 = h;
	base = MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (base==NULL) {throw std::runtime_error("mapviewoffile");}	
	this->addr = base;
	this->mod = 3;
}

void MemoryMappedFile::load_rw(const char *fname) {
	if (is_open()) throw std::runtime_error("still open");
	void *base;
	HANDLE fd,h;
	fd = CreateFile(fname,(GENERIC_READ|GENERIC_WRITE),FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);

	if (fd==INVALID_HANDLE_VALUE) {
		throw std::runtime_error("createfile");
	}
	this->h1 = fd;
	h = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (h==NULL) {throw std::runtime_error("createfilemapping");}	
	this->h2 = h;
	base = MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (base==NULL) {throw std::runtime_error("mapviewoffile");}	
	this->addr = base;
	this->mod = 3;
}

void MemoryMappedFile::resize_rw(size_t new_len) {
	if (is_open()) throw std::runtime_error("still open");
	void *base;
	if (new_len != this->len) return ;
	UnmapViewOfFile(this->addr);
	CloseHandle(this->h2);
	HANDLE fd = this->h1, h;

	LARGE_INTEGER li;
	li.QuadPart = new_len;
	if (!SetFilePointerEx(fd, li, NULL, FILE_BEGIN)) {
		throw std::runtime_error("cannot set pointer ");
	}
	if (!SetEndOfFile(fd)) {
		throw std::runtime_error("cannot set end of file");
	}
	h = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (h==NULL) {
		throw std::runtime_error("createfilemapping");
	}	
	this->h2 = h;
	base = MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (base==NULL) {
		throw std::runtime_error("mapviewoffile");
	}
	this->addr = base;
}




#else

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif


void MemoryMappedFile::load_r(const char *fname) {
	if (is_open()) throw std::runtime_error("still open");
	int fd;
	size_t len;
	struct stat statbuf;
	caddr_t base;

	stat(fname, &statbuf);
	len = statbuf.st_size;
	fd = open(fname,O_RDONLY);
	if (fd == -1) {
		throw std::runtime_error((std::string("open file: \'") + fname + "\'").c_str());
	}
	base = (caddr_t)mmap(0,len,PROT_READ,MAP_SHARED,fd,0);
	if (base==(caddr_t)-1) {
		throw std::runtime_error("mmap1\n");
	}
	this->addr = (void *)base;
	this->fd = fd;
	this->len = len;
	this->mod = 1;
}

void MemoryMappedFile::create_rw(const char *fname, uint64_t len) {
	if (is_open()) throw std::runtime_error("still open");
	int fd;
	caddr_t base;
	char c;

	fd = open(fname,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR);
	if (fd == -1) { throw std::runtime_error(std::string("create_w: open ") + fname);}
	fchmod(fd, 0644);
	int result = lseek(fd, len-1, SEEK_SET);
	if (result == -1) {
		::close(fd);/* free(m); */
		throw std::runtime_error("Error calling lseek() to 'stretch' the file");
    }
	result = write(fd, "", 1);
	if (result != 1) {
		::close(fd);/*free(m); */
		throw std::runtime_error("Error writing last byte of the file");
	}
	lseek(fd, 0, SEEK_SET);

	base = (caddr_t)mmap(0,len,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	if (base==MAP_FAILED) {::close(fd); throw std::runtime_error("mymmap_w");}
	this->addr = (void *)base;
	this->fd = fd;
	this->len = len;
	this->mod = 3;
}

void MemoryMappedFile::load_rw(const char *fname) {
	if (is_open()) throw std::runtime_error("still open");
	int fd;
	caddr_t base;
	char c;
	struct stat statbuf;
	//m = (MMAP*) malloc(sizeof(*m));
	//if (m==NULL) {throw std::runtime_error("mymmap_w malloc");  }
	fd = open(fname,O_RDWR);
	if (fd == -1) { throw std::runtime_error("mymmap_w: open1");}
	fchmod(fd, 0644);
	stat(fname, &statbuf);
	len = statbuf.st_size;
	base = (caddr_t)mmap(0,len,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	if (base==MAP_FAILED) {::close(fd);  throw std::runtime_error("mymmap_w");}
	this->addr = (void *)base;
	this->fd = fd;
	this->len = len;
	this->mod = 3;
	//return m;
}

void MemoryMappedFile::resize_rw(size_t new_len){
	if (is_open()) throw std::runtime_error("still open");
	unsigned char *buf;
	long l,bs;
	caddr_t base;
	struct stat statbuf;

	if (msync(this->addr, this->len, MS_ASYNC)) {
		throw std::runtime_error("msync");
		
	}
	if (munmap(this->addr,this->len)==-1) {
		throw std::runtime_error("mymremap 1:");
	}

	if (this->fd != -1) {
		if (new_len > this->len) {
			bs = 1<<16;
			buf = (unsigned char*) malloc(bs);
			if (buf==NULL) {throw std::runtime_error("mymremap: malloc buf");  }
			for (l=0; l<bs; l++) buf[l] = 0;

			if (lseek(this->fd, this->len, SEEK_SET) == -1) {
				throw std::runtime_error("mymremap: lseek"); 
			}
			l = new_len - this->len;
			while (l > 0) {
				long s;
				//      printf("write %ld \r",l);  fflush(stdout);
				s = min(l, bs);
				if (write(this->fd, buf, s) != s) {
					throw std::runtime_error("mymremap: write"); 
				}
				l -= s;
			}
			free(buf);

		} else {
			if (ftruncate(this->fd, new_len)) {
				throw std::runtime_error("mymremap: ftruncate"); 
			}
		}
	}
	if (this->fd == -1) {
		base = (caddr_t)mmap(0,new_len,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,this->fd,0);
	} else {
		base = (caddr_t)mmap(0,new_len,PROT_READ | PROT_WRITE, MAP_SHARED,this->fd,0);
	}
	if (base==MAP_FAILED) {
		throw std::runtime_error("mymremap: mmap");
	}
	this->addr = (void *)base;
	this->len = new_len;
	//return m->addr;
}

void MemoryMappedFile::close() {
	if (this->mod != 0) {
		if (munmap(this->addr,this->len)==-1) {
			throw std::runtime_error("munmap 1:");
		}
		::close(this->fd);
		this->addr = NULL;
		this->mod = 0;
	}
}                
#endif
}//namespace

