cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS sdarray.cpp sdarray_sml.cpp deltaarray.cpp huffarray.cpp 
gamma_arr.cpp
remap_dt.cpp
sdarray_th.cpp
sdarray_rl.cpp
sdarray_blk.cpp
_experiment/sdarray_blk2.cpp
sdarray_c.cpp
vlen_array.cpp
)

set(HEADERS sdarray.h sdarray_sml.h deltaarray.h intarray.h 
huffarray.h gamma_arr.h
remap_dt.h
blkarray.hpp diffarray.hpp
transform_utils.hpp
contextfree_models.hpp
sdarray_th.h
sdarray_zero.h
sdarray_blk.h
sdarray_rl.h
_experiment/sdarray_blk2.h
sdarray_c.h
sdarray_interface.h
runlen.h
vlen_array.h
)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray codec utils)
#add_sources(mscdsa ${SRCS} ${HEADERS})


add_test_files(sda_test.cpp)

add_test_files(codearr_test.cpp)

add_test_files(intarr_test.cpp)

add_test_files(sda_th_test.cpp)

add_test_exec(t_sda_c FILES sda_c_test.cpp LIBS mscdsa gtest)
