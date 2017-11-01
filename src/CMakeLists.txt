cmake_minimum_required(VERSION 2.8)

add_library(csvf reader.cpp writer.cpp split.cpp combine.cpp)
target_include_directories(csvf PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(csvf PUBLIC csvf_header PUBLIC ${Boost_LIBRARIES})

