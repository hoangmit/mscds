MSCDS Library                         {#mainpage}
============


MSCDS is a framework and collection of succinct and compressed data structures. MSCDS framework defines abstractions for memory, files, parameters. The framework helps developers to re-use and compose existing data structures, and simplify the implementation of new data structures. It also helps to adapt the data structures for different systems and environments.

MSCDS implements a set of well-known succinct and compressed data structures in the research literature. It divides into a few smaller sets of related libraries.

* BitArray: Data structures related to BitArray with rank/select operations
* IntArray: Compressed arrays of Integers
* Wavarray: Two-dimension data structures
* Tree: Tree related data stuctures
* Application data structures: cwig, cbed

## Framework concepts ##


The MSCDS framework defines rules that all data structures within MSCDS need to follow. This help the data structures to co-existing and can be re-used and composed to more complex one. The framework also provides basic building blocks that data structures can use to simplify the implementation.

## Data structure types ##

### Independent and auxiliary

Data structures implemented in MSCDS library are usually catagorized into independent data structure and auxliary data structure.
* *Independent data structure*: these are data structure that have all the information of within. The data structure can be saved and loaded from/to disk.
* *Auxiliary data structure*: A data structure that depends on other data structures is called auxiliary data structure. It stores some additional data to support/enhance operations of an existing data structure.

For example, given `BitArray` data structure, `Rank25pAux` is an auxiliary data structures that stores one additional table to support rank query. When you load a auxiliary data structure, you need to supply the original `BitArray` with it. If you give it the wrong main data structure, the behavior is undefined. For convenience, we also supply `Rank25p` data structure, it is an independent data structure that contains both a `BitArray` and a `Rank26pAux`.

A independent data structure needs to implement two functions `save(OutArchive&)` and `load(InpArchive&)`. An auxiliary data structure is required two functions `save_aux(OutArchive&)` and `load_aux(InpArchive&, MainDataSturcutre&)`. One data structure can implement both the normal data structure and auxiliary data structure interfaces.

### Builder and Query

MSCDS focus on supporting compressed "static" data structure. Once the data structure is built, it is no longer modifiable. Therefore, the data structures usually implemented in two classes: *Query* class and *Builder* class. As the name suggest, *Builder* class stores data to build the data structure, while *Query* class provides the query interface. For example, `Rank25pBuilder` class is used to build `Rank25p` data structure.


## Memory models ##

The memory model is used to save (aka. serialize) and load the data structure. MSCDS library supports a few memory models. All the memory models are sub-class of `InpArchive` or `OutArchive`.

Models for Data structure builder class
* Memory
* Disk

Memory models for Query class:
* RAM memory:
* Mapped memory:
* Cached memory:
 * Disk file
 * Cached remote file
   * HTTP remote file
   * FTP remote file

## Data structure documentations ##


### Organization ###
MSCDS source code is orgainized into several directories. Each directory contains code for related data structures or functions. The following table list some important directories and files. (The full list of directories, classes and functions can be found in the Doxygen generated document.)

|Directory | Description | Example |
|----------|-------------|---------|
| `utils`    | Utility classes and functions | <ul><li>`file_utils.h`: file and directory utilities</li><li>`str_utils.h`: string parsing utilities</li><li>`hash_utils.h`: hash functions</li><li>etc.</li></ul>  |
| `codec`    | Basic encoding and decoding | <ul><li>`deltacoder.h`</li> <li>`huffman_code.h`</li> <li>`arithmetic_code.hpp`</li> <li>etc.</li></ul> |
| `mem`      | Memory management (on RAM, and disk) | <ul><li>`file_archive2.h`</li> <li>`fmap_archive2.h`</li> </ul> |
| `bitarray` | Bitwise operations and BitArray classes | <ul><li>`bitop.h`: Bitwise functions</li><li>`bitarray.h`: BitArray classes</li><li>`rank6p.h`: One of the Rank/Select classes</li><li>`rrr3.h`: Compressed BitArray</li><li>etc.</li></ul> |
| `intarray` | Integer array classes | <ul><li>`sdarray_sml.h`: SDArray data structure</li><li>`huffarray.h`: Huffman compressed array</li><li>etc.</li></ul>|
| `wavarray` | Wavelet tree data structure  | <ul><li>`wat_array.h`: Wavelet tree array</li><li>`count2d.h`: Wavelet tree with count support</li></ul> |
| `tree`     | Tree related data structures | <ul><li>`RMQ_sct.h`</li><li>`CartesianTree.h`</li><li>`BP_bits.h`</li><li>etc.</li> |
| `cwig2`     | CWig data structure          | `cwig2.h`: CWig format functions |



### User guide:

Getting started
* [C++ knowledge prerequisite](_1_cpp_guide.md)
* [Install MSCDS](_0a_install_mscds.md)

Memory models:
* [Save data strucure to file](_1_save_to_file.md)

Data structures for array of bits:
* [BitArray](_2_bitarray.md)
* [RankSelect](_3_rankselect.md)

Data structure for array of integers
* [SDArray](_4_sdarray.md)
* [Wavelet array](_5_wavelet_array.md)


Application
* [CWig](cwig.md)
* CBed

### Developer guide:

Memory models:
* [Low level memory management](low_lvl_mem.md)
