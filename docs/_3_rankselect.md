# Rank/Select data structures

## Concepts ##
Rank Select data structures are a set of data structures that support at least two operations <tt>rank</tt> and <tt>select</tt> on a bit vector.

Given an array <math>A[0..(m - 1)]</math>, the rank and select is defined as follows:
* <math>rank(c, p)</math> is the number of times value <math>c</math> occurs in the sub-array <math>A[0..(p-1)]</math>
* <math>select(c, i)</math> returns index <math>p</math> such that <math>A[p] = c</math> and <math>rank(c, p) = i</math>. Note that <math>i</math> starts from <math>0</math>.

== Classes ==

We have a few rank/select data structures. They are:
* `Rank25p`: uses additional 25% space of the original bit vector for indexing.
* `Rank6p`: uses additional 6% space.
* `Rank3p`: uses additional 6% space.
* `RRR2`: is compressed rank/select data structure.

The relative speed of the structures are: <tt>to be completed</tt>

## Examples ##

````````cpp
#include "bitarray/rank6p.h"
//...

  using namespace mscds;
  BitArray v = BitArrayBuilder::create(100);
  //... set the values for the BitArray
  Rank6p rst;
  Rank6pBuilder::build(v, &rst);
  std::cout << rst.rank(10) << std::endl;
````````

## References ##
````````cpp
/// the interface for ranks/select data structures in "bitarray/rankselect.h"

/** counts the number of 1 in the range from 1 to (p-1) */
uint64_t rank(uint64_t p) const;
/** counts the number of 0 in the range from 1 to (p-1) */
uint64_t rankzero(uint64_t p) const;
/** the position of the (r+1)-th 1-value (from left to right) */
uint64_t select(uint64_t r) const;
/** the position of the (r+1)-th 0-value (from left to right) */
uint64_t selectzero(uint64_t r) const;
/** returns the number of 1 in the whole array */
uint64_t one_count() const { return onecnt; }
/** returns the length of the array */
uint64_t length() const { return bits.length(); }
/** returns the value p-th bit in the bit array */
bool access(uint64_t p) const;
````````



