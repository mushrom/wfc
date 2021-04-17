cmake_minimum_required(VERSION 3.13)
project(wfc-test VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE)
	message(WARNING "CMAKE_BUILD_TYPE not set, defaulting to Release")
	set(CMAKE_BUILD_TYPE Release)
endif()


add_executable(tile-gen src/stbi.c example/thing.cpp)
target_include_directories(tile-gen PUBLIC include)
