cmake_minimum_required(VERSION 2.6)

project(gntf)

set(SRCS chrfmt.cpp gntf.cpp stringarr.cpp
RLSum6.cpp 
SampledSum.h
poly_vals.cpp nintv.cpp)

set(HEADERS chrfmt.h gntf.h stringarr.h 
RLSum6.h 
SampledSum.cpp
rlsum_int.h poly_vals.h nintv.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(gntf ${SRCS} ${HEADERS})
target_link_libraries(gntf mscdsa)

add_executable(t_gntf gntf_test.cpp)
target_link_libraries(t_gntf gntf)
set_target_properties(t_gntf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_rlsum rlsum_test.cpp)
target_link_libraries(t_rlsum intarray  gntf ${COMMON_LIBS})
set_target_properties(t_rlsum PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(tbm_gntf benchmark_gntf.cpp)
target_link_libraries(tbm_gntf gntf)