cmake_minimum_required(VERSION 2.6)

project(cwig)

include_directories(${Boost_INCLUDE_DIRS})

set(SRCS chrfmt.cpp cwig.cpp
RLSum6.cpp 
SampledSum.cpp
rank_vals.cpp
poly_vals.cpp
intv/nintv.cpp
)

set(HEADERS chrfmt.h cwig.h 
RLSum6.h 
SampledSum.h
rank_vals.h
rlsum_int.h poly_vals.h
float_precision.h
intv/nintv.h
)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(cwig ${SRCS} ${HEADERS})
target_link_libraries(cwig mscdsa remote_file fusionarray)
set_property(TARGET cwig PROPERTY FOLDER "Apps")

#add_test_files(intv/intv_tests.cpp)
#intv/intv_rand_gen.hpp
 
add_test_exec(t_cwig FILES cwig_test.cpp rlsum_test.cpp val_arr_test.cpp intv/intv_tests.cpp LIBS cwig cwig2 utils)

add_test_exec(tbm_cwig FILES benchmark_cwig.cpp LIBS cwig)


add_subdirectory(${PROJECT_SOURCE_DIR}/ctools/)
