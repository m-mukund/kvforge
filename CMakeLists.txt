cmake_minimum_required(VERSION 3.10)

project(KVForge VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tell CMake where to look for header files (for later)
include_directories(include)

# Point CMake to the new location of main.cpp
add_executable(KVForge src/main.cpp src/ThreadPool.cpp src/KVStore.cpp src/StorageManager.cpp src/ProtocolParser.cpp)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(KVForgeTests tests/test_kvstore.cpp tests/test_aof.cpp src/KVStore.cpp src/StorageManager.cpp src/ProtocolParser.cpp)

target_link_libraries(KVForgeTests GTest::gtest_main)

# Register tests with CMake
include(GoogleTest)
gtest_discover_tests(KVForgeTests)