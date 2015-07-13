# Low level memory management #


## Introduction ##

This memory manager provides a simple set of APIs to access multiple types of memory namely: local RAM memory, local file, and remote file. 
The memory manager handles file access, address mapping, data caching and network communication to simplify and unify the process. Different memory types can be read/written through a basic API which handle the detailed operations. The manager also provide advanced API with more type details for user who prefer optimizations. 

There are four main abstract classes: Data structures, Memory region, Allocator and Archive. (Each abstract concepts may have multiple implementations, but shares the same interface.) 
The interactions between these classes are:
`Data structure` uses `Memory Region`s for memory access.
Blank `MemoryRegion`s are created from `Allocator`.
`Memory region`s can be saved or loaded into `Archive`.

<div style="text-align:center;">
<img src="./mem_images/diagram1.png" alt="Drawing" style="width: 300px;"/>
</div>

Memory region is an abstract of a physical RAM memory region. It is consecutive memory region in RAM, which is similar to a fixed array of characters.
Allocator is an abstraction of C language's `malloc()` function.
Archive is an abstraction of a file on hard disk.
The abstraction layer deals with:
file caching schemes,
file location and access mode,
e.g. remote or local file.

### Examples ###

<div style="text-align:center;">
<img src="./mem_images/Picture1.png" alt="Drawing" style="width: 420px;"/>
</div>


<div style="text-align:center;">
<img src="./mem_images/Picture2.png" alt="Drawing" style="width: 420px;"/>
</div>


<div style="text-align:center;">
<img src="./mem_images/Picture2.png" alt="Drawing" style="width: 420px;"/>
</div>



### Data structure ###


The convention in the MSCDS library is each data structure is implemented in two classes: `Builder` class and `Query` class. The builder implements the data structure creation algorithms. It also serves as the container for temporary data. Query class implements query algorithms from compacted data.

The Builder class requests new memory pointers from Allocator. It also implements two methods: `add()` and `build(Query*)`. The `add()` function store data to the allocated memory. The `build()` function pass the memory pointers to the Query class.

The Query class implements specific query functions. It also need to implements `save(OutArchive&)` and `load(InpArchive&)` functions. The `save()` function writes memory to the `Archive`, while the `load` function read memory from the input archive.

Below is an example of a simple data structure that only store a string and allow access it characters.

There are two classes the Builder class construct the string. The Query class allows user to read from the string. The `example()` function is how the functions are used. There are a few classes that were not introduced namely `DynamicMemRegionPtr`, `StaticMemRegionPtr`, `IFileArchive2` and `OFileArchive2`. They are different implementations of Memory Region and Archive. They will be mentioned in the subsequence sub-sections. In this example, we want to highlight the conventional flow of  interactions of data structure classes in the `example()` function.

~~~~~~~~~cpp
#include "framework/mem_models.h"
#include "framework/archive.h"
#include "mem/local_mem.h"

using namespace mscds;

/// Example: Builder class for SString
class SStringBuilder {
public:
	SStringBuilder();
	// append to the current string
	void add(const char* s, unsigned int len);
	// build query data structure
	void build(SString* out);
private:
	DynamicMemRegionPtr data;
};

class SString {
public:
	// the the i-th character
	char get(unsigned int i) const;
	// string length
	unsigned int length() const;
	// save to file
	void save(OutArchive& ar) const;
	// load from file
	void load(InpArchive& ar);
	//... (add some functions later) ...
private:
	friend class SStringBuilder;
	mutable StaticMemRegionPtr ptr;
};

// ... (implementation) ...

void example() {
	SStringBuilder bd;
	bd.add("testing a", 9);
	// create a string
	SString original;
	bd.build(&original);
	// create output file
	OFileArchive2 fo;
	fo.open_write("/tmp/string.bin");
	// write to file
	original.save(fo);
	fo.close();

	SString local;
	IFileArchive2 fi;
	// read from local file
	fi.open_read("/tmp/string.bin");
	local.load(fi);
	fi.close();

	// check if length is correct
	assert(9 == original.length());
	assert(original.length() == local.length());
}
~~~~~~~~~


## Allocator ##

Purposes:
* Allocate memory and return a Static/DynamicMemRegion
* Convert from one type of memory to another type of memory


~~~~~~~~~cpp
using namespace mscds;
struct DynamicMemRegionInterface {
	StaticMemRegionPtr allocStaticMem(size_t size);
	DynamicMemRegionPtr allocDynMem(size_t init_sz = 0);
	StaticMemRegionPtr convert(const DynamicMemRegionAbstract& ptr);
}
~~~~~~~~~

Currently only memory is implemented in `mem/local_mem.h`.



## Memory Region ##

Types:

 * StaticMemRegion: cannot change size 
 * DynamicMemRegion: can resize the region


Each has 2 sets of functions

 * Basic functions
 * Advanced functions

### Basic functions (Static Memory Region)

~~~~~~~~~cpp
//small one time access
uint64_t getword(size_t wp) const
char getchar(size_t i) const

void setword(size_t wp, uint64_t val)
void setchar(size_t i, char c)

void read(size_t i, size_t rlen, void* dst) const
void write(size_t i, size_t wlen, const void* dst)

// return true to continue, return false to break
typedef std::function<bool(const void* p, size_t len)> CallBack;
void scan(size_t i, size_t len, CallBack cb) const
size_t size() const
void close()
~~~~~~~~~

### Advanced functions (Static Memory Region)

3 memory access types:
 * API_ACCESS: The memory is remote, and data is not cached. There is no consistent address of the data.
 * MAP_ON_REQUEST: There is a persistent cache of the remote data in memory. User can read data directly using pointer access in request region.
 * FULL_MAPPING: this is local memory or fully cached data.

Memory Alignment Types: A1 (byte), A2 (2 bytes), A4 (4 bytes), A8 (1 word)

Endianness Type
 * LITTLE_ENDIAN_ACCESS
 * BIG_ENDIAN_ACCESS

Modification types:
 * StaticMemRegion: cannot change size 
 * DynamicMemRegion: can resize the region

~~~~~~~~~cpp
MemoryAlignmentType alignment() const
MemoryAccessType memory_type() const
//only little-endian is supported at the moment
EndiannessType endianness_type() const

// FULL_MAPPING and MAP_ON_REQUEST
const void* get_addr() const
// MAP_ON_REQUEST
bool request_map(size_t start, size_t len)
~~~~~~~~~


Example:

### Dynamic Memory Region API ###

All function from StaticMemoryRegion and…


Stack like model. The changes are all at the tail of the region.

~~~~~~~~~cpp
void resize(size_t size);
void append(char c);
void append(uint64_t word);
void append(const void * ptr, size_t len);
void append(StaticMemRegionAbstract& other);
~~~~~~~~~


## Archive ##


Archive stores:
 * Meta data: small fixed size data e.g. length, number of one count
 * Annotations: names of variables
 * Data structure class
 * Memory regions: Big chunk of memory

Archive is either:
 * Input archive (InpArchive) : write data to disk
 * Output archive (OutArchive): read data from disk/network to memory

Many sub-types of archives
 * Sub-types shares the interfaces but stores the data differently
 * The subtypes return different types of MemoryRegion

Example of sub-types
 * Type1: data and meta-data is stored in order of declarations
 * Type2: meta-data is stored in consecutive locations for faster loading
 * Statistic: only measure sizes

<div style="text-align:center;">
<img src="./mem_images/archive_types.png" alt="Drawing" style="width: 420px;"/>
</div>


All files with same layout version are compatible.

|Archive name|	Layout version|	Memory Cache|	Description|
|------------|:--------------:|:-----------:|--------------|
|IFileArchive1, OFileArchive1,IFileArchive2, OFileArchive2|	1, 2|	no|	Write to file, but load everything in memory|
|OFileMapArchive1,OFileMapArchive1|1,2|	full|	file mapping cache, size unlimited, managed by OS|
|OFileCCacheArchive|  _ |  | partial| (UNDONE)  custom cache with limited size.|
|OMemArchive, IMemArchive|	1|	full|	In memory stream, no file is written|
|OSizeEstArchive|	_	|full	|In memory to estimate data structure size|
|OClassInfoArchive|	_	|full	|In memory, store annotation and meta-data to debug
|OBindArchive|	_	|no	|duplicate data to two streams
|RemoteArchive1,RemoteArchive2|	1,2|	full|	Access remote file on HTTP server. This uses a mirror file to cache remote file.|
|RemoteArchiveNoCache|	2|	no|	Remote file without any cache|
|RemoteArchiveMemCache|	2|	partial	|(UNDONE) Remote file with a shared memory cache|


~~~~~~~~~cpp
class SString;
class SStringBuilder;
using namespace mscds;

/// Example: Builder class for SString
class SStringBuilder {
public:
	SStringBuilder();
	/// append to the current string
	void add(const char* s, unsigned int len);
	/// build query data structure
	void build(SString* out);
private:
	DynamicMemRegionPtr data;
};

/// Example: a class use StaticMemRegionPtr, save and load from Archive
class SString {
public:
	/// count the number of 'a' character in string
	unsigned int count_a() const;
	/// the the i-th character
	char get(unsigned int i) const;
	/// the the i-th character (i.e. same as 'get'), 
	/// using different APIs to read data
	char get_adv(unsigned int i) const;
	/// string length
	unsigned int length() const;
	/// empty constructor
	SString();
	/// save to file
	void save(OutArchive& ar) const;
	/// load from file
	void load(InpArchive& ar);
private:
	void init() { number_a = count_a(); }
	friend class SStringBuilder;

	unsigned int number_a;
	mutable StaticMemRegionPtr ptr;
};

SStringBuilder::SStringBuilder() {
	LocalMemAllocator a;
	data = a.allocDynMem();
}

inline void SStringBuilder::add(const char *s, unsigned int len) {
	data.append(s, len);
}

void SStringBuilder::build(SString *out) {
	LocalMemAllocator a;
	out->ptr = a.convert(data);
	out->init();
	data.close();
}

inline unsigned int SString::count_a() const {
	size_t c = 0;
	for (unsigned int i = 0; i < length(); ++i)
		if (get(i) == 'a') c++;
	return c;
}

inline char SString::get(unsigned int i) const { assert(i < length()); return ptr.getchar(i); }

inline char SString::get_adv(unsigned int i) const {
	assert(i < length());
	char *tmp = NULL;
	if (ptr.memory_type() == FULL_MAPPING) {
		tmp = (char*)ptr.get_addr();
		return tmp[i];
	} else if (ptr.memory_type() == MAP_ON_REQUEST) {
		ptr.request_map(i, 1);
		tmp = (char*)ptr.get_addr();
		return tmp[i];
	} else {
		char ch;
		// fall back to basic API
		ptr.read(i, 1, &ch);
		return ch;
	}
}

inline SString::SString() { number_a = 0; }

inline unsigned int SString::length() const { return ptr.size(); }

inline void SString::save(OutArchive &ar) const {
	ar.startclass("string");
	ar.var("count_a").save(number_a);
	ar.var("string_mem").save_mem(ptr);
	ar.endclass();
}

inline void SString::load(InpArchive &ar) {
	ar.loadclass("string");
	ar.var("count_a").load(number_a);
	ptr = ar.var("string_mem").load_mem_region();
	ar.endclass();
	if (count_a() != number_a) throw std::runtime_error("data mismatched");
}

void example_str1() {
	SStringBuilder bd;
	bd.add("testing a", 9);
	// create a string
	SString original;
	bd.build(&original);
	// create output file
	OFileArchive2 fo;
	fo.open_write("C:/temp/string.bin");
	// write to file
	original.save(fo);
	fo.close();

	// declare empty string
	SString local, remote;
	IFileArchive2 fi;
	// read from local file
	fi.open_read("C:/temp/string.bin");
	local.load(fi);
	fi.close();

	// check if length is correct
	assert(9 == original.length());
	assert(original.length() == local.length());

	// load from remote file
	RemoteArchive2 rfi;
	rfi.open_url("http://localhost/string.bin");
	remote.load(rfi);
	rfi.close();

	assert(local.length() == remote.length());
}
~~~~~~~~~
