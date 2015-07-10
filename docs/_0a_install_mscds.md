# Install MSCDS


## Install dependency

MSCDS requires the following components:
* C\++ compiler that supports C\++11 feature, for example: gcc 4.7+, clang 3.2+ or Visual C 2013+
* [Boost library](http://www.boost.org/) to compile
* [CMake](http://www.cmake.org/) to create makefile
* [Optional] [Mercurial version control](https://mercurial.selenic.com/) to mange the changes



### Install dependency in Linux

On RedHat/Centos system
~~~~~~~~bash
sudo yum install \
  gcc-c++ \
  boost-devel boost-static \
  cmake \
  mercurial
~~~~~~~~

On ubuntu system:
~~~~~~~~bash
sudo apt-get install \
  build-essential \
  libboost-all-dev \
  cmake \
  mercurial
~~~~~~~~


### Install dependency from source

If you don't have the dependency and cannot install them using system package management. You can install them from source. For GCC, follow [this instructions](http://gcc.gnu.org/wiki/InstallingGCC). The instructions to compile Boost and cmake are availble on their respective website.

Setup the paths for the new components in these environment variables.
~~~~~~~bash
export CXX=<g++_binary>
export CC=<gcc_binary>
export BOOST_ROOT=<path_to_boost>
export LD_LIBRARY_PATH=<additional_library_path>:$LD_LIBRARY_PATH
export LIBRARY_PATH=<additional_library_path>:$LIBRARY_PATH
export CPLUS_INCLUDE_PATH=<additional_include_path>:$CPLUS_INCLUDE_PATH
export C_INCLUDE_PATH=<additional_include_path>:$C_INCLUDE_PATH
~~~~~~~

### Install on Windows

Detailed instructions will be available later.
* Download the components
* Set environment variables

## Obtain MSCDS source code

If you have mecurial, you can clone the code with:
~~~~~~~~bash
# clone project using mercurial
hg clone https://bitbucket.org/hmm01/mscds
~~~~~~~~

Alternatively, you can download the source code from: https://bitbucket.org/hmm01/mscds


## Compile MSCDS

~~~~~~~~bash
cd mscds
mkdir build
cd build
cmake ..
make
~~~~~~~~

## Use mscds


### CMake

If you a bigger project, you can try to use [CMake](http://www.cmake.org/) to generate a native make file.

Create a "CMakeLists.txt" file for your project:
~~~~~~~~~~~~~~~cmake
cmake_minimum_required(VERSION 2.6)
project(myproject_name)

### MSCDS_LIBRARY path, change according to your location
set(MSCDS_PATH "somewhere") # e.g. "$ENV{HOME}/mscds"
set(MSCDS_LIB ${MSCDS_PATH}/build/lib/libmscdsa.a)
include_directories(${MSCDS_PATH}/)
include_directories(${MSCDS_PATH}/framework)

###compiler flag for your program
add_definitions(-DNDEBUG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -std=c++0x")

### Boost library (Required by certain functions)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) #library variable ${Boost_LIBRARIES}

### Multithreads (Optional)
# find_package(Threads REQUIRED) #library variable: ${CMAKE_THREAD_LIBS_INIT}

###
set(SRCS
file1.cpp
file2.cpp
)

set(HEADERS
file1.h
file2.h
)

add_executable(your_program_name ${SRCS} ${HEADERS})
target_link_libraries(your_program_name ${MSCDS_LIB})
~~~~~~~~~~~~~~~

### Makefile


For GCC, use ["-I" option](http://gcc.gnu.org/onlinedocs/gcc/Directory-Options.html) to specify the include directory; use ["-L", "-l" options](http://gcc.gnu.org/onlinedocs/gcc/Link-Options.html) to specify the library path and library name.

For example:
~~~~~~~~~~~~~~~bash
g++
~~~~~~~~~~~~~~~

