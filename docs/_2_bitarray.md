# BitArray
This tutorial introduces the usage of BitArray library inside MSCDS. For the reference of BitArray see [[BitArray reference]].
For the installing MSCDS library see [[abc]].


## Create BitArray


Create a BitArray

``````````cpp
#include "bitarray/bitarray.h"
#include <iostream>
using namespace std;
int main() {
  using namespace mscds;
  // create a bit array of length 4
  BitArray b = BitArrayBuilder::create(4);
  // assign the value to some positions
  b.setbit(0, true);
  b.setbit(1, false);
  b.setbit(2, false);
  b.setbit(3, true);
  // get the bit values at a position
  cout << b[0] << endl;
}
``````````

Note that BitArray work like a managed memory pointer. i.e.

``````````cpp
BitArray bits; /* point to the actual bit array.
Note that this variable does not duplicate the memory, it stores a pointer */
BitArray a = BitArray::create(...);
BitArray b = a; /* only one region of memory is allocated */
a.setbit(i, true); // both a and b are affected.
``````````

Since the size (length) of a bit array is fixed once the array is created. If you want to build a bitarray with unknown size, you can use "OBitStream" class. For example:

``````````cpp
#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"
// ....
  OBitStream os;
  os.put0(); // false
  os.put1(); // true
  os.put1();
  os.put0();
  os.close();
  BitArray b;
  os.build(&b);
``````````

## Access BitArray


``````````cpp
// get one bit
int i = 3;
cout << b[i] << endl;
// get 3 bits from bitarray b, start at 1, length 3
// the result is returned in a 64-bit unsigned int 
// the 0-th bit of v is the first bit of the result
// the 1-th bit of v is the second bit of the result
uint64_t v = b.bits(1,3);
``````````

## Save BitArray into a file

``````````cpp
#include "mem/file_arhcive2.h" // in older version use "file_archive.h"
//....
  BitArray b = BitArrayBuilder::create(len)
  save_to_file(b, "file_name.bin");
``````````

## Method Reference

``````````cpp
/** set one bit at "bitindex" with "value" */
void setbit(size_t bitindex, bool value);
/** set a few bits start at "bitindex" with length "len", the values of those bits 
are given by the "value" input word */
void setbits(size_t bitindex, uint64_t value, unsigned int len);
/** set 64 bits start at 64*"pos" with the input word "val" */
void setword(size_t pos, uint64_t val);
/** fill the array */
void fillzero();
void fillone();
/** clear the bitarray and free memory */
void clear();

/** return the length of the array */
size_t length() const;
/** return the number of words */
size_t word_count() const;
/** count how many one inside the array */
uint64_t count_one() const;

/** read "len" bits from the array start at "bitindex" */
uint64_t bits(size_t bitindex, unsigned int len) const;
/** read one bit */
bool bit(size_t bitindex) const;
/** read one bit (operator version) */
bool operator[](size_t i) const;
/** read one byte (8 bits) at "pos"*8 */
uint8_t byte(size_t pos) const;
/** read one word (64 bits) */
uint64_t word(size_t pos) const;

/** load the BitArray from InpArchive */
InpArchive& load(InpArchive& ar);
/** save the BitArray to OutArchive */
OutArchive& save(OutArchive& ar) const;
/** convert to string for debug or display */
std::string to_str() const;
``````````


