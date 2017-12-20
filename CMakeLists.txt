cmake_minimum_required(VERSION 2.8)
project(csvf)

if(POLICY CMP0022)
  cmake_policy(SET CMP0022 NEW)
endif()

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS iostreams filesystem system program_options)

include(CTest)

add_subdirectory(include/csvf)
add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(test)
