# Wavelet array #

Given an array `A[0..(m - 1)]`, the rank and select is defined as follows:
* `rank(c, p)` is the number of times value `c` occurs in the sub-array `A[0..(p-1)]`
* `select(c, i)` returns index `p` such that `A[p] = c` and `rank(c, p) = i`. Note that `i` starts from `0`.

## Examples ##

````````cpp
#include <iostream>
#include <vector>  // for vector
#include "wat_array.h"
using namespace mscds;
//...
  std::vector<uint64_t> list;
  list.push_back(100);
  list.push_back(10);
  WatBuilder bd;
  bd.build(list, &wq);

  std::cout << wq.rank(10,2) << std::endl;
````````
## References ##

````````cpp
// in file <wavarray/wat_array.h>

/** return the value at position 'p' */
uint64_t access(uint64_t p) const;
/** counts the number of times that value 'c' occurs from sub-array [0..(p-1)] */
uint64_t rank(uint64_t c, uint64_t p) const;
/** returns the (r+1)-th position of value 'c' in the array */
uint64_t select(uint64_t c, uint64_t r) const;
/** count the number of times that any value less than 'c' occurs in the sub-array [0..(p-1)] */
uint64_t rankLessThan(uint64_t c, uint64_t p) const;
/** counts the number of times that any value greater than 'c' occurs in the sub-array [0..(p-1)] */
uint64_t rankMoreThan(uint64_t c, uint64_t p) const;
/** returns all the values of rank, rankLessThan, rankMoreThan in one query */
void rankAll(uint64_t c, uint64_t pos,
	uint64_t& rank,  uint64_t& rank_less_than, uint64_t& rank_more_than) const;

uint64_t kthValue(uint64_t begin_pos, uint64_t end_pos, uint64_t k, uint64_t & pos) const;
/** returns the value of the maximum/minimun value and one of its position in the sub-array [begin_pos..end_pos] */
uint64_t maxValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const;
uint64_t minValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const;
````````


