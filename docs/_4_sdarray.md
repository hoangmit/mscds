# SDArray #

This tutorial introduces the usage of SDArray library in MSCDS.

There are two variants of SDArray. It is recommended to use SDArraySml.

## Model ##

SDArray is an array of non-negative integers $A[0..(m-1)]$. 
It provides the following operations:
*  access any element $A[i]$ in constant time 
*  compute $prefix\_sum(i)= \sum_{k=0}^{i-1} A[k]$ in constant time. (note that $prefix\_sum(0)=0$)
*  compute $rank(p)$ returns the minimal index i such that $prefix\_sum(i) \geq p$ in logarithmic time.


Let $n=\sum_{k=1}^{m}A[i]$. The SDArray should uses about $m \log_2 (n/m) + 2m + o(n)$ bits.

## Create SDArray ##


````````cpp
#include "intarray/sdarray_sml.h"
using namespace mscds;
//...
  SDArraySmlBuilder bd;
  bd.add(1);
  bd.add(0);
  bd.add(2);

  SDArraySml sda;
  bd.build(&sda);
  std::cout << sda.lookup(0) << std::endl;
````````

## References ##

````````cpp
/** returns the value of A[i] */
uint64_t lookup(const uint64_t i) const;

/** returns the value of A[i] and  prev_sum=prefix_sum(i) */
uint64_t lookup(const uint64_t i, uint64_t& prev_sum) const;

/** return the value of prefix_sum(i) */
uint64_t prefixsum(size_t i) const ;

/** return the value of rank(p) */
uint64_t rank(uint64_t p) const;

/** clear the array (length becomes 0) */
void clear();

/** save and load functions */
void save(OutArchive& ar) const;
void load(InpArchive& ar);

/** to string for debug */
std::string to_str(bool psum) const;

/** returns the sum of all the elements in the array */
uint64_t total() const;

/** counts the number of elements in the array */
uint64_t length() const;
````````


