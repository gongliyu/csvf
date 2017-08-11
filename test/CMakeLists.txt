cmake_minimum_required(VERSION 2.8)

include(BoostTestHelpers.cmake)
message("${Boost_LIBRARIES}")
add_boost_test(reader.cpp
    DEPENDENCY csvf ${Boost_LIBRARIES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
    )
#add_test(NAME reader COMMAND ReaderTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
