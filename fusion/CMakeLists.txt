cmake_minimum_required(VERSION 2.6)

project(fusionarray)


set(SRCS 
blkgroup_array.cpp
sdarray_block.cpp
)

set(HEADERS
blkgroup_array.h
sdarray_block.h
fuse_blk_model.h
inc_ptrs.h
inc_ptrs2.h
inc_ptrs3.h
ps_access_blk.h
)

add_library(fusionarray ${SRCS} ${HEADERS})
target_link_libraries(fusionarray bitarray codec utils intarray)


add_test_exec(blkgroup_array_test FILES blkgroup_array_test.cpp LIBS fusionarray utils)
