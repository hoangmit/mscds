cmake_minimum_required(VERSION 2.6)

project(utils)

set(SRCS
cache_ls.cpp
str_utils.cpp
file_utils.cpp
utils.cpp
debug.cpp
param.cpp
modp_numtoa.cpp
cache_table.cpp
rand_data.cpp
benchmark.cpp
md5.cpp
)


set(HEADERS
cache_ls.h
str_utils.h
file_utils.h
utils.h
debug.h
utest.h
param.h
modp_numtoa.h
cache_table.h
rand_data.h
benchmark.h
md5.h
)

include_directories(${GTEST_INCLUDE_DIRS})


add_library(utils ${SRCS} ${HEADERS})
target_link_libraries(utils ${GTEST_LIBRARY})
#add_sources(mscdsa ${SRCS} ${HEADERS})


add_test_files(cache_table_test.cpp md5_test.cpp)
#add_executable(t_cache_table cache_table_test.cpp)
#target_link_libraries(t_cache_table utils)
#set_target_properties(t_cache_table PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

