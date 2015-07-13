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

The memory model is used to save (aka. serialize) and load the data structure. MSCDS library supports a few memory models.

Models for Data structure builder
* Memory
* Disk
Memory models for Query data structures:
* RAM memory:
* Mapped memory:
* Cached memory:
 * Disk file
 * Cached remote file
   * HTTP remote file
   * FTP remote file

## Data structure documentations ##

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
