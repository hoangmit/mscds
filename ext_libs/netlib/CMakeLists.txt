#            Copyright (c) Dean Michael Berris 2010.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 2.6)
project(CPP-NETLIB)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTI_THREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package( Boost 1.44.0 REQUIRED system regex date_time thread filesystem chrono )
find_package( OpenSSL )
find_package( Threads )

MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARIES}")

#set(CMAKE_VERBOSE_MAKEFILE true)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DBOOST_NETWORK_DEBUG)
endif()

if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if (${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if (Boost_FOUND)
    add_definitions(-DBOOST_DATE_TIME_NO_LIB)
    if (MSVC)
      add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    endif(MSVC)
    if (WIN32)
      add_definitions(-D_WIN32_WINNT=0x0501)
    endif(WIN32)
    include_directories(${Boost_INCLUDE_DIRS})
    #enable_testing()
    add_subdirectory(libs/network/src)
    #add_subdirectory(libs/network/test)
    #add_subdirectory(libs/network/example)
endif(Boost_FOUND)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

