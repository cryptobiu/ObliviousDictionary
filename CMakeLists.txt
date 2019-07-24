cmake_minimum_required(VERSION 3.13)
project(obliviousDictionary)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -mbmi2 -msse4.2 -march=native -m64 -pthread -fpermissive")
INCLUDE_DIRECTORIES($ENV{HOME} $ENV{HOME}/libscapi/install/include xxHash
        $ENV{HOME}/libscapi/install/include/OTExtensionBristol )
link_directories(/usr/ssl/lib/ $ENV{HOME}/libscapi/install/lib xxHash/cmake_unofficial)

set(SOURCE_FILES main.cpp obliviousDictionary.cpp)


add_executable(obliviousDictionary main.cpp obliviousDictionary)

TARGET_LINK_LIBRARIES(obliviousDictionary ntl.a libxxhash.so
        $ENV{HOME}/libscapi/libscapi.a gmp
        libboost_system.a pthread crypto dl)