# This is the cmake file. It describes how to build our app
# The documentation for how to build these can be found here:
# http://www.cmake.org/cmake/help/cmake-2-8-docs.html

# CMake likes us to put this here in case someone tries to read this file with an older version of cmake
cmake_minimum_required(VERSION 2.8) # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:cmake_minimum_required

# Sets the name of the project. Additionally this sets the variables <projectName>_BINARY_DIR and <projectName>_SOURCE_DIR to the respective values.
project ("ElladanJson") # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:project


file(GLOB_RECURSE source "src/*.cpp")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wshadow ")
SET_SOURCE_FILES_PROPERTIES( ${source}  PROPERTIES LANGUAGE CXX )
add_library(ElladanJson SHARED ${source} )
add_library(ElladanJsonSta STATIC ${source} )
set_target_properties(ElladanJsonSta PROPERTIES OUTPUT_NAME ElladanJson)


find_library(ElladanHelper ElladanHelper)
target_link_libraries(ElladanJson ElladanHelper)
target_link_libraries(ElladanJsonSta ElladanHelper)

# Export header
install(DIRECTORY src/ DESTINATION include/elladan/json
        FILES_MATCHING PATTERN "*.h")
install(TARGETS ElladanJson LIBRARY DESTINATION lib)
install(TARGETS ElladanJsonSta ARCHIVE DESTINATION lib)

enable_testing()

# Main Json structure
add_executable(jsonTest test/JsonTest.cpp)
target_link_libraries(jsonTest ElladanHelper ElladanJsonSta)
add_test(jsonTest jsonTest)

# json -> string serialization
add_executable(jsonSer test/JsonSerializer.cpp)
target_link_libraries(jsonSer ElladanHelper ElladanJsonSta)
add_test(jsonSer jsonSer)

add_executable(jsonDeser test/JsonDeserializer.cpp)
target_link_libraries(jsonDeser ElladanHelper ElladanJsonSta)
add_test(jsonDeser jsonDeser)

# Json -> Bson serialization
add_executable(bsonSer test/BsonSerializer.cpp)
target_link_libraries(bsonSer ElladanHelper ElladanJsonSta)
add_test(jbonSer bsonSer)

add_executable(bsonDeser test/BsonDeserializer.cpp)
target_link_libraries(bsonDeser ElladanHelper ElladanJsonSta)
add_test(bsonDeser bsonDeser)

# Pointer & reference
add_executable(pointerTest test/PointerTest.cpp)
target_link_libraries(pointerTest ElladanHelper ElladanJsonSta)
add_test(pointerTest pointerTest)

# Schema
add_executable(schemaTest test/SchemaTest.cpp)
target_link_libraries(schemaTest ElladanHelper ElladanJsonSta)
add_test(schemaTest schemaTest)
