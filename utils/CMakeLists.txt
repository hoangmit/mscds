cmake_minimum_required(VERSION 2.6)

project(utils)

set(SRCS
cache_ls.cpp
str_utils.cpp
file_utils.cpp
utils.cpp
debug.cpp
)


set(HEADERS
cache_ls.h
str_utils.h
file_utils.h
utils.h
debug.h
utest.h
)


#message(${Boost_LIBRARIES})
#add_library(utils ${SRCS} ${HEADERS})
#target_link_libraries(utils)
add_sources(mscdsa ${SRCS})


