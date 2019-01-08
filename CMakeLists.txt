# This is the cmake file. It describes how to build our app
# The documentation for how to build these can be found here:
# http://www.cmake.org/cmake/help/cmake-2-8-docs.html

# CMake likes us to put this here in case someone tries to read this file with an older version of cmake
cmake_minimum_required(VERSION 2.8) # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:cmake_minimum_required

# Sets the name of the project. Additionally this sets the variables <projectName>_BINARY_DIR and <projectName>_SOURCE_DIR to the respective values.
project ("ElladanJson") # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:project


file(GLOB_RECURSE source "src/*.cpp")
file(GLOB_RECURSE header "src/*.h")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wshadow ")
SET_SOURCE_FILES_PROPERTIES( ${source} ${header} PROPERTIES LANGUAGE CXX )
add_library(ElladanJson STATIC ${source} )

find_library(ElladanHelper ElladanHelper)
target_link_libraries(ElladanJson ElladanHelper)

# Export header
install(DIRECTORY src/ DESTINATION include/elladan/json
        FILES_MATCHING PATTERN "*.h")
install(TARGETS ElladanJson ARCHIVE DESTINATION lib)

message("cmake version " ${CMAKE_C_COMPILER_VERSION})


enable_testing()

add_executable(jsonTest test/JsonTest.cpp)
target_link_libraries(jsonTest ElladanHelper ElladanJson)
add_test(jsonTest jsonTest)

add_executable(jsonSerTest test/JsonSerializer.cpp)
target_link_libraries(jsonSerTest ElladanHelper ElladanJson)
add_test(jsonSerTest jsonSerTest)

add_executable(jsonDeserTest test/JsonDeserializer.cpp)
target_link_libraries(jsonDeserTest ElladanHelper ElladanJson)
add_test(jsonDeserTest jsonDeserTest)

add_executable(bsonSerTest test/BsonSerializer.cpp)
target_link_libraries(bsonSerTest ElladanHelper ElladanJson)
add_test(bsonSerTest bsonSerTest)

