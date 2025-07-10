include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/googletest)

file(GLOB_RECURSE TEST_INCLUDE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

assign_source_group(${TEST_INCLUDE_FILES})
assign_source_group(${TEST_SOURCE_FILES})

add_executable(PunkEngineTest ${TEST_INCLUDE_FILES} ${TEST_SOURCE_FILES} ${PUNK_ENGINE_ALLLIB_SOURCES})
target_include_directories(PunkEngineTest PRIVATE
	  ${DEPENDENCIES_DIRECTORY}/boost/pfr/include
    ${DEPENDENCIES_DIRECTORY}/boost/preprocessor/include
    ${DEPENDENCIES_DIRECTORY}/asio/asio/include
    ${DEPENDENCIES_DIRECTORY}/async_simple
    ${DEPENDENCIES_DIRECTORY}/DirectXMath/Inc
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(PunkEngineTest gtest gtest_main)