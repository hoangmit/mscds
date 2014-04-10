cmake_minimum_required(VERSION 2.8)

#pre-project options

#change compiler setting here
#set(CMAKE_C_COMPILER "/opt/gcc-4.7.2/bin/gcc")
#set(CMAKE_CXX_COMPILER "/opt/gcc-4.7.2/bin/g++")

#set(BOOST_ROOT "")

set(PYTHON_INCLUDE_DIR "/usr/include/python2.6")
set(PYTHON_LIBRARY "/usr/lib64/libpython2.6.so")

#set(JAVA_INCLUDE_PATH)
#set(JNI_INCLUDE_DIRS)

#-Options-----------------------------------------------------------------------------------------

project(mscds)
set(VERSION 0.1.1)

set (CMAKE_VERBOSE_MAKEFILE OFF)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
	  "Choose the type of build, options are: None Debug Release."
	  FORCE)
endif (NOT CMAKE_BUILD_TYPE)

### Build PYTHON and JAVA API extension settings
option(BUILD_EXT_API "Build Python and Java API" OFF)

if (WIN32)
	set(BUILD_EXT_API OFF)
endif()

MESSAGE(STATUS "C Compiler version: ${CMAKE_C_COMPILER_VERSION}")
MESSAGE(STATUS "C++ Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(GCC_COMPATIBLE_COMPILER ON)
	MESSAGE(STATUS "Found GCC compatible compiler")
else()
	set(GCC_COMPATIBLE_COMPILER OFF)
endif()

if (BUILD_EXT_API)
    set(BUILD_DYNAMIC_CODE ON) # for dynamic library in Linux
	set(LINK_STATIC_LIBS OFF)
else()
    set(BUILD_DYNAMIC_CODE OFF)
	set(LINK_STATIC_LIBS ON)
endif()
if (BUILD_DYNAMIC_CODE AND CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	MESSAGE(STATUS "Added flags:  -fPIC")
endif()
if (CMAKE_COMPILER_IS_GNUCC AND (LINK_STATIC_LIBS))
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
	MESSAGE(STATUS "Added linker flags:  -static-libgcc -static-libstdc++")
endif()

#-------------------------------------------------------------------------------------------------
### Common compiler flags
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(cmake/function.cmake)
fix_compiler_flags() #defined inside function.cmake

if(GCC_COMPATIBLE_COMPILER)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O3 -g")
elseif(MSVC)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
	# 'expression': unreferenced local variable
	#add_definitions(/wd4101)
	# 'conversion' conversion from 'type1' to 'type2', possible loss of data
	add_definitions(/wd4244)
	add_definitions(/wd4267)
	# 'expression' : signed/unsigned mismatch
	add_definitions(/wd4018)
	add_definitions(/D_VARIADIC_MAX=10)
	add_definitions(-D_WIN32_WINNT=0x0501)
	#add_definitions(/arch:SSE)
endif()

MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include(cmake/MergeStaticLibraries.cmake)

### setup directories-------------------------------------------
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
   CACHE PATH "Single Directory for all Executables.")
set(TEST_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
	CACHE PATH "Directory for testings executables")
set(MODULE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/modules
	CACHE PATH "Directory for python modules")
#  .exe = runtime
#  .dll = runtime
#  .lib = archive  (either import lib or static lib)
#  .a   = archive
#  .so  = library
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)


#---Find BOOST------------------------------------------------------------------------------

set(Boost_USE_MULTI_THREADED ON)
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_NO_SYSTEM_PATHS ON)
if(MSVC)
	set(Boost_USE_STATIC_LIBS ON)
endif()
#set(Boost_USE_STATIC_RUNTIME OFF)
if (BUILD_DYNAMIC_CODE)
    set(Boost_USE_STATIC_LIBS OFF)
else()
    set(Boost_USE_STATIC_RUNTIME ON)
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost)

if(NOT Boost_FOUND)
	set(Boost_NO_BOOST_CMAKE OFF)
	set(Boost_NO_SYSTEM_PATHS OFF)
	find_package(Boost REQUIRED)
endif()

include_directories(${Boost_INCLUDE_DIRS})
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARIES}")

#------------------------------------------------------------------------------------------------

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/gtest) #${PROJECT_BINARY_DIR}/ext_libs/gtest
set(GTEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/gtest/include)
set(GTEST_LIBRARY gtest)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/snappy)
set(SNAPPY_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/snappy)
set(SNAPPY_LIBRARY snappy)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/zlib)
set(ZLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/zlib)
set(ZLIB_LIBRARY zlibstatic)


add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/netlib)
set(NETLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/netlib)
set(NETLIB_LIBRARIES cppnetlib-uri cppnetlib-client-connections)

include_directories(${GTEST_INCLUDE_DIRS})


#---Interal Utilities-----------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/runtests.py ${TEST_OUTPUT_DIRECTORY}/runtests.py COPYONLY)
	
include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}/framework)

add_subdirectory(${PROJECT_SOURCE_DIR}/mem)
add_subdirectory(${PROJECT_SOURCE_DIR}/utils)

set(COMMON_LIBS mem utils)

########################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/extcodec)

add_subdirectory(${PROJECT_SOURCE_DIR}/bitarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/codec)
add_subdirectory(${PROJECT_SOURCE_DIR}/intarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/wavarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/tree)
add_subdirectory(${PROJECT_SOURCE_DIR}/string)

set(CORE_LIBS utils mem bitarray codec intarray wavarray tree string)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
   CACHE PATH "Single Directory for all Libraries")  
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
   CACHE PATH "Single Directory for all static libraries.")


MERGE_STATIC_LIBRARIES(mscdsa "${CORE_LIBS}")
#set_target_properties(mscdsa PROPERTIES
#         LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#set_target_properties(mscdsa PROPERTIES
#	ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/lib)
#get_property(mscdsa_location TARGET mscdsa PROPERTY LOCATION)
#MESSAGE(" Location =" ${mscdsa_location})
#get_property(mscdsa_SRCS GLOBAL PROPERTY mscdsa_SRCS)
#add_library(mscdsa STATIC ${mscdsa_SRCS})
#add_library(mscdsa STATIC common/mscdsa.h common/mscdsa.cpp)
set(MSCDS_VERSION 0.1)

add_subdirectory(${PROJECT_SOURCE_DIR}/unittests)

MERGE_STATIC_LIBRARIES(extcodeca "extcodec;${SNAPPY_LIBRARY};${ZLIB_LIBRARY}")

add_subdirectory(${PROJECT_SOURCE_DIR}/remote_file)
MERGE_STATIC_LIBRARIES(remote_access "remote_file;cppnetlib-client-connections;cppnetlib-uri;cppnetlib-server-parsers")


########################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/export_lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/cwig)
add_subdirectory(${PROJECT_SOURCE_DIR}/cbed)
#add_subdirectory(${PROJECT_SOURCE_DIR}/mfm)
#add_subdirectory(${PROJECT_SOURCE_DIR}/gtags)

##========================================================
export(TARGETS mscdsa FILE "${PROJECT_BINARY_DIR}/mscdsaTargets.cmake")
export(PACKAGE mscdsa)

# Create the FooBarConfig.cmake and FooBarConfigVersion files
#file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")