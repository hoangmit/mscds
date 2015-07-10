Save data strucure to file
==========================

Data structures that need to use the file save/load service are required to implement two methods `save`, and `load`.

```````````cpp
#include "framework/archive.h"
//.....
class DataStructreX {
public:
  void save(mscds::OutArchive& ar) const;
  void load(mscds::InpArchive& ar);
};
```````````

## Save and Load methods ##

Example:
```````````cpp
#include "mem/file_archive2.h"
using namespace mscds;
//..

OFileArchive2 fo;
ar.open_write("path/to/file");
BitArray a;
SDArraySml b;
//...
a.save(fo);
b.save(fo);
//...
ar.close();

BitArray c;
SDArraySml d;
IFileArchive2 fi;
fi.open_read("path/to/file");
fi.close();
//...
c.load(fi);
d.load(fi);
//...
fi.close();
```````````

You can measure the disk size of the data structure that implemented save/load by using
```````````cpp
#include "mem/info_archive.h"
using namespace mscds;
//...
DataStructure ds;
size_t s = estimate_data_size(ds)
```````````

## Writing save/load method for your data structure ##


Example:

```````````cpp
class DataStructreX {
public:
  void save(mscds::OutArchive& ar) const;
  void load(mscds::InpArchive& ar);
private:
  SDArraySml arr;
  BitArray bits;
  unsigned _size;
};
void DataStructreX::save(mscds::OutArchive& ar) const {
  // declare class name, and class version
  ar.startclass("class_name_data_structure_x", 1); 
  // use ".var()" to declare sub-datastructure name or variable name
  // it is optional, but is recommended for debugging and XML export
  arr.save(ar.var("array"));
  // normal integer variable can be saved directly
  ar.var("size_variable").save(_size);

  bits.save(ar.var("bit_vector"));
  // close the class declaration
  ar.endclass();
}

void DataStructreX::load(mscds::InpArchive& ar) {
  // load method needs to match the order of the save method
  int class_version = ar.loadclass("class_name_data_structure_x");
  arr.load(ar.var("array"));
  ar.var("size_variable").load(_size);
  bits.load(ar.var("bit_vector"));
  ar.endclass();
}
```````````

## API references ##


The API of the `OutArchive`.
```````````cpp
virtual OutArchive& var(const std::string&) { return * this; }
virtual OutArchive& var(const char*) { return *this; }
virtual OutArchive& annotate(const std::string&) { return * this; }

virtual OutArchive& startclass(const std::string&, unsigned char version = 1) { return *this;  };
virtual OutArchive& endclass() { return *this; };

virtual OutArchive& save(uint32_t v) { return save_bin(&v, sizeof(v)); }
virtual OutArchive& save(int32_t v)  { return save_bin(&v, sizeof(v)); }
virtual OutArchive& save(uint64_t v) { return save_bin(&v, sizeof(v)); }
virtual OutArchive& save(int64_t v)  { return save_bin(&v, sizeof(v)); }
virtual OutArchive& save_bin(const void* ptr, size_t size) = 0;

virtual OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4) = 0;
virtual OutArchive& add_mem_region(const void* ptr, size_t size) = 0;
virtual OutArchive& end_mem_region() = 0;
```````````

API of the `InpArchive` interface.
```````````cpp
virtual InpArchive& var(const std::string&) { return *this; }
virtual InpArchive& var(const char*) { return *this; }
virtual unsigned char loadclass(const std::string& name) { return 0; };
virtual InpArchive& endclass() { return *this; };

virtual InpArchive& load(uint32_t& v) { return load_bin(&v, sizeof(v)); }
virtual InpArchive& load(int32_t& v) { return load_bin(&v, sizeof(v)); }
virtual InpArchive& load(uint64_t& v) { return load_bin(&v, sizeof(v)); }
virtual InpArchive& load(int64_t& v) { return load_bin(&v, sizeof(v)); }

virtual InpArchive& load_bin(void* ptr, size_t size) = 0;

// BoundedMemRegion is defined in mem_models.h
virtual StaticMemRegionPtr load_mem_region() = 0;

virtual size_t ipos() const = 0;
virtual bool eof() const = 0;
virtual void close() {}
```````````
