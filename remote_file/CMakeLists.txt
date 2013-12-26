cmake_minimum_required(VERSION 2.6)

project(netlib)
set(VERSION 0.1.1)

#find_package(Boost)
FIND_PACKAGE(Boost 1.50 COMPONENTS thread system  REQUIRED )

#LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${NETLIB_INCLUDE_DIRS})

#ADD_EXECUTABLE(netlib_test netlib_test.cpp)
#TARGET_LINK_LIBRARIES(netlib_test ${Boost_LIBRARIES} cppnetlib-uri cppnetlib-client-connections) # cppnetlib-server-parsers

SET(SRCS remote_file.cpp strptime.cpp http_client.cpp 
ext_bitmap.cpp memmapfile.cpp
remote_archive.cpp
)

SET(HEADERS remote_file.h strptime.h
remote_file_impl.h http_client.h
ext_bitmap.h memmapfile.h
remote_archive.h
)


ADD_LIBRARY(remote_file ${SRCS} ${HEADERS})

TARGET_LINK_LIBRARIES(remote_file ${Boost_LIBRARIES} ${NETLIB_LIBRARIES} utils) 

ADD_EXECUTABLE(remote_file_example remote_file_example.cpp)
TARGET_LINK_LIBRARIES(remote_file_example remote_file bitarray intarray mem)
set_property(TARGET remote_file_example PROPERTY FOLDER "Tests")
