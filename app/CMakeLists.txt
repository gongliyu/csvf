add_executable(csvf_split csvf_split.cpp)
target_link_libraries(csvf_split csvf ${Boost_LIBRARIES})

add_executable(csvf_combine csvf_combine.cpp)
target_link_libraries(csvf_combine csvf ${Boost_LIBRARIES})
