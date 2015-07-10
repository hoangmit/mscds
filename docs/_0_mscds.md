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


* *Independent data structure*: these are data structure that have all the information of within. The data structure can be saved and loaded from/to disk.
* *Auxiliary data structure*: Data structure that depends on other data structures. A data structure is called auxiliary data structure, if it store some additional data to support/enhance operations of an existing data structure. For example, Rank25p auxiliary data structures store one additional table to support rank query for an existing BitArray. When you load a auxiliary data structure, you need to supply the main data structure. If you give it the wrong main data structure, the behavior is undefined.

A independent data structure needs to implement two functions `save(OutArchive&)` and `load(InpArchive&)`. An auxiliary data structure is required two functions `save_aux(OutArchive&)` and `load_aux(InpArchive&, MainDataSturcutre&)`.

One data structure implements both the normal data structure and auxiliary data structure interfaces.

## Data structure stage ##

MSCDS focus on supporting compressed "static" data structure. Once the data structure is built, you can no longer modify it. Therefore, the data structures usually have two stages:
* Building stage:
* Completed stage:

## Memory models ##

Models for Data structure builder
* Memory
* Disk

We aim to support the following memory models for Query data structures:
* RAM memory:
* Mapped memory:
* Cached memory:
 * Disk file
 * Cached remote file
   * HTTP remote file
   * FTP remote file

## Getting started ##

[C++ knowledge prerequisite](_1_cpp_guide.md)

[Install MSCDS](_0a_install_mscds.md)

## Data structure documentations ##
[Save data strucure to file](_1_save_to_file.md)
Data structures for array of bits:
* [BitArray](_2_bitarray.md)
* [RankSelect](_3_rankselect.md)

Data structure for array of integers
* [SDArray](_4_sdarray.md)
* [Wavelet array](_5_wavelet_array.md)

Design
* [Low level memory management](low_lvl_mem.md)

Application 
* [CWig](cwig.md)
* CBed


