Save data strucure to file
==========================

MSCDS provides services for data structure to save/load from external memory. To be able to use the service, data structures are required to implement two methods `save`, and `load`. The signature of these methods are:

```````````cpp
#include "framework/archive.h"
//.....
class DataStructreX {
public:
  void save(mscds::OutArchive& ar) const;
  void load(mscds::InpArchive& ar);
};
```````````

## Usage examples ##

The following is an example of using the methods with File Archive object:
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

Shortcuts without explicit declaration of archive objects;
````cpp
#include "mem/shortcuts.h"

//...
mscds::BitArray a;
mscds::save_to_file(a, "path/to/file");
mscds::BitArray b;
mscds::load_from_file(b, "path/to/file");
````

Example on BitArray:
```````````cpp
#include "bitarray/bitarray.h"
#include "mem/shortcuts.h"
#include <iostream>
using namespace std;
void create_and_save() {
  using namespace mscds;
  // create a bit array of length 4 using its Builder class
  BitArray b = BitArrayBuilder::create(4);
  // assign the value to some positions
  b.setbit(0, true);
  b.setbit(1, false);
  b.setbit(2, false);
  b.setbit(3, true);
  // get the bit values at a position
  save_to_file(b, "path/to/file");
}

void load_and_use() {
  using namespace mscds;
  BitArray bx;
  load_from_file(bx, "path/to/file");
  for (unsigned i = 0; i < bx.length(); ++i)
    cout << bx[i];
  cout << endl;
}
```````````


You can measure the disk size of the data structure that implemented save/load by using
```````````cpp
#include "mem/info_archive.h"

DataStructure ds;
//...
size_t sz = mscds::estimate_data_size(ds)
```````````

## Writing save/load method for your data structure ##


Example on how to implement save/load methods:

```````````cpp
using namespace mscds;

//Declarations
class DataStructreX {
public:
  void save(mscds::OutArchive& ar) const;
  void load(mscds::InpArchive& ar);
private:
  //Using existing data structures
  SDArraySml arr;
  BitArray bits;
  unsigned _size;
};
void DataStructreX::save(mscds::OutArchive& ar) const {
  // declare class name
  ar.startclass("class_name_data_structure_x");
  // use ".var()" to declare sub-datastructure name or variable name
  // it is optional, but is recommended for debugging and XML export
  arr.save(ar.var("array"));
  // normal integer variable can be saved directly
  ar.var("size_variable").save(_size);

  //using other data structure save function
  bits.save(ar.var("bit_vector"));
  // close the class declaration
  ar.endclass();
}

void DataStructreX::load(mscds::InpArchive& ar) {
  // load method needs to match the order of the save method
  ar.loadclass("class_name_data_structure_x");
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

virtual OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4);
virtual OutArchive& add_mem_region(const void* ptr, size_t size);
virtual OutArchive& end_mem_region();
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

To develop new Archive class, developer often only requires to implements `save_bin`, `load_bin` and `load_mem_region` methods. See existing code for more information.

