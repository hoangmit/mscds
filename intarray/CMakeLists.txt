cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS sdarray.cpp sdarray_sml.cpp deltaarray.cpp huffarray.cpp 
gamma_arr.cpp
remap_dt.cpp
)

set(HEADERS sdarray.h sdarray_sml.h deltaarray.h intarray.h 
huffarray.h gamma_arr.h
remap_dt.h
blkarray.hpp diffarray.hpp
transform_utils.hpp
contextfree_models.hpp
)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray codec utils)
#add_sources(mscdsa ${SRCS} ${HEADERS})



add_test_files(sda_test.cpp)


add_test_files(codearr_test.cpp)

add_test_files(intarr_test.cpp)

