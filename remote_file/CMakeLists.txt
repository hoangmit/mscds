cmake_minimum_required(VERSION 2.6)

project(remote_file)
set(VERSION 0.1.1)

#find_package(Boost)
FIND_PACKAGE(Boost 1.50 COMPONENTS thread system chrono date_time filesystem regex REQUIRED)

#LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

find_package(OpenSSL)
if (OPENSSL_FOUND)
    add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${NETLIB_INCLUDE_DIRS})

#ADD_EXECUTABLE(netlib_test netlib_test.cpp)
#TARGET_LINK_LIBRARIES(netlib_test ${Boost_LIBRARIES} cppnetlib-uri cppnetlib-client-connections) # cppnetlib-server-parsers

SET(SRCS remote_file.cpp strptime.cpp http_client.cpp http_headers.cpp
ext_bitmap.cpp memmapfile.cpp
remote_archive1.cpp
remote_archive2.cpp
remote_file_impl.cpp
)

SET(HEADERS remote_file.h strptime.h
remote_file_impl.h http_client.h http_headers.h
ext_bitmap.h memmapfile.h
remote_archive1.h
remote_archive2.h
error.h
)

ADD_LIBRARY(remote_file ${SRCS} ${HEADERS})

if (OPENSSL_FOUND)
	TARGET_LINK_LIBRARIES(remote_file ${Boost_LIBRARIES} ${NETLIB_LIBRARIES} utils ${OPENSSL_LIBRARIES})
else()
	TARGET_LINK_LIBRARIES(remote_file ${Boost_LIBRARIES} ${NETLIB_LIBRARIES} utils) 
endif()

MESSAGE(STATUS "lib in use: ${Boost_LIBRARIES}")

ADD_EXECUTABLE(remote_file_example remote_file_example.cpp)
TARGET_LINK_LIBRARIES(remote_file_example remote_file mscdsa cwig ${Boost_LIBRARIES})
set_property(TARGET remote_file_example PROPERTY FOLDER "Tests")

