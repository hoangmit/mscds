cmake_minimum_required(VERSION 2.6)

project(fusionarray)


set(SRCS 
block_mem_mng.cpp
generic_struct.cpp
sdarray_block.cpp
)

set(HEADERS
block_mem_mng.h
sdarray_block.h
sdarray_blk_hints.h
codec_block.h
inc_ptrs.h
inc_ptrs2.h
inc_ptrs3.h
ps_access_blk.h
fixsize_block.h
polymorphic_blk.h
generic_struct.h
)

add_library(fusionarray ${SRCS} ${HEADERS})
target_link_libraries(fusionarray bitarray codec utils intarray)


add_test_exec(blkgroup_array_test FILES blkgroup_array_test.cpp sdarray_benchmarks.cpp fused_sdarray_test.h  LIBS fusionarray utils)

add_test_exec(blk_mem_test FILES block_mem_test.cpp LIBS fusionarray utils mem)
