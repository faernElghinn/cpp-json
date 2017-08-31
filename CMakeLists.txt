# This is the cmake file. It describes how to build our app
# The documentation for how to build these can be found here:
# http://www.cmake.org/cmake/help/cmake-2-8-docs.html

# CMake likes us to put this here in case someone tries to read this file with an older version of cmake
cmake_minimum_required(VERSION 2.8) # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:cmake_minimum_required

# Sets the name of the project. Additionally this sets the variables <projectName>_BINARY_DIR and <projectName>_SOURCE_DIR to the respective values.
project ("elladanJson") # http://www.cmake.org/cmake/help/cmake-2-8-docs.html#command:project

file(GLOB_RECURSE source "src/*.cpp")
file(GLOB_RECURSE header "src/*.h")

set(CMAKE_CXX_FLAGS "-std=c++11 ")
SET_SOURCE_FILES_PROPERTIES( ${source} ${header} PROPERTIES LANGUAGE CXX )
add_library(ElladanJson STATIC ${source} )

find_library(ElladanHelper ElladanHelper)
target_link_libraries(ElladanJson ElladanHelper)

# Export header
install(DIRECTORY src/ DESTINATION include/elladan/json
        FILES_MATCHING PATTERN "*.h")
install(TARGETS ElladanJson ARCHIVE DESTINATION lib)




enable_testing()

add_executable(jsonTest test/JsonTest.cpp)
target_link_libraries(jsonTest ElladanHelper ElladanJson)
add_test(jsonTest jsonTest)

add_executable(serializer2JsonTest test/JsonSerializer2String.cpp)
target_link_libraries(serializer2JsonTest ElladanJson ElladanHelper)
add_test(serializer2JsonTest serializer2JsonTest)

add_executable(serializerFromJsonTest test/JsonSerializerFromString.cpp)
target_link_libraries(serializerFromJsonTest ElladanJson ElladanHelper)
add_test(serializerFromJsonTest jsonTest)

add_executable(bsonFromBson test/BsonSerializerFromJson.cpp)
target_link_libraries(bsonFromBson ElladanJson ElladanHelper)
add_test(bsonFromBson bsonFromBson)

add_executable(bson2Bson test/BsonSerializer2Json.cpp)
target_link_libraries(bson2Bson ElladanJson ElladanHelper)
add_test(bson2Bson bson2Bson)



