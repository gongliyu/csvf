cmake_minimum_required(VERSION 2.8)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
include(BoostTestHelpers.cmake)
add_boost_test(reader_test.cpp
    DEPENDENCY csvf ${Boost_LIBRARIES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
add_boost_test(writer_test.cpp
    DEPENDENCY csvf ${Boost_LIBRARIES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data
    )
add_boost_test(split_test.cpp
    DEPENDENCY csvf ${Boost_LIBRARIES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data
    )
add_boost_test(options_test.cpp
    DEPENDENCY csvf ${Boost_LIBRARIES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data
    )
  
#add_test(NAME reader COMMAND ReaderTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
