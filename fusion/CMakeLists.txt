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

add_executable(blkgroup_array_test blkgroup_array_test.cpp)
target_link_libraries(blkgroup_array_test fusionarray utils ${COMMON_LIBS})
set_target_properties(blkgroup_array_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET blkgroup_array_test PROPERTY FOLDER "Tests")