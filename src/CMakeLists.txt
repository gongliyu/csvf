cmake_minimum_required(VERSION 2.8)

add_library(csvf reader.cpp)
target_link_libraries(csvf PRIVATE csvf_header)

