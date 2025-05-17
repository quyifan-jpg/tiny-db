cmake_minimum_required(VERSION 3.16)
project(smallkv)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-march=native -ffast-math)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-Werror -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
# Add this near the top of your CMakeLists.txt or before targets
# Ensure -g is also present (usually handled by CMAKE_BUILD_TYPE=Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")

option(SMALLKV-UNITTEST "build unit-test" ON)
option(SMALLKV-BENCHMARK "build benchmark" ON)

# --- Add this line ---
include(CTest)
# --------------------

# gtest
find_package(GTest REQUIRED)
find_package(Threads REQUIRED) # Needed for GTest and potentially your code
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake/absl")  # 关键修改
set(Abseil_DIR "/usr/local/lib/cmake/absl")     
find_package(absl REQUIRED COMPONENTS 
    crc32c 
    crc_internal 
    raw_logging_internal 
    base
)
# spdlog
find_package(spdlog REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})



include_directories(src)

file(GLOB_RECURSE SRC
        src/cache/*.cpp
        src/cache/*.h
        src/compress/*.cpp
        src/compress/*.h
        src/config/*.cpp
        src/config/*.h
        src/db/*.cpp
        src/db/*.h
        src/file/*.cpp
        src/file/*.h
        src/filter/*.cpp
        src/filter/*.h
        src/log/*.cpp
        src/log/*.h
        src/memory/*.cpp
        src/memory/*.h
        src/memtable/*.cpp
        src/memtable/*.h
        src/recovery/*.cpp
        src/recovery/*.h
        src/sstparser/*.cpp
        src/sstparser/*.h
        src/table/*.cpp
        src/table/*.h
        src/utils/*.cpp
        src/utils/*.h
        src/wal/*.cpp
        src/wal/*.h
        src/main.cpp)

file(GLOB_RECURSE SRC_WITHOUT_MAIN
        src/cache/*.cpp
        src/cache/*.h
        src/compress/*.cpp
        src/compress/*.h
        src/config/*.cpp
        src/config/*.h
        src/db/*.cpp
        src/db/*.h
        src/file/*.cpp
        src/file/*.h
        src/filter/*.cpp
        src/filter/*.h
        src/log/*.cpp
        src/log/*.h
        src/memory/*.cpp
        src/memory/*.h
        src/memtable/*.cpp
        src/memtable/*.h
        src/recovery/*.cpp
        src/recovery/*.h
        src/sstparser/*.cpp
        src/sstparser/*.h
        src/table/*.cpp
        src/table/*.h
        src/utils/*.cpp
        src/utils/*.h
        src/wal/*.cpp
        src/wal/*.h)

# Create a library from the smallkv code
add_library(smallkv_lib ${SRC_WITHOUT_MAIN})
target_link_libraries(smallkv_lib PRIVATE 
    absl::crc32c 
    absl::crc_internal 
    absl::raw_logging_internal 
    absl::base
)
# Create the smallkv executable
add_executable(smallkv src/main.cpp)
target_link_libraries(smallkv PRIVATE smallkv_lib)

# Add debug information for GDB
set_target_properties(smallkv PROPERTIES
    CMAKE_BUILD_TYPE Debug
    COMPILE_FLAGS "-g3 -O0"
)

# --- Add this section for becnhmark ---
if(SMALLKV-BENCHMARK)
    message(STATUS "Building benchmark")

    # Find all .cpp files in the benchmark directory
    file(GLOB BENCHMARK_SOURCES "test/benchmark/*.cpp")

    # Add an executable for the benchmarks
    add_executable(benchmarks_smallkv ${BENCHMARK_SOURCES})

    # Link the benchmark executable against your library and GTest
    target_link_libraries(benchmarks_smallkv PRIVATE smallkv_lib GTest::gtest GTest::gtest_main Threads::Threads)

    # Include the GTest header directory
    target_include_directories(benchmarks_smallkv PRIVATE ${GTEST_INCLUDE_DIRS})
else()
    message(STATUS "Skipping benchmarks")
endif()

# --- Add this section for tests ---
if(SMALLKV-UNITTEST)
    message(STATUS "Building unit tests")

    # Find all .cpp files in the test directory
    file(GLOB TEST_SOURCES "test/*.cpp")


    # Add an executable for the tests
    add_executable(tests_smallkv ${TEST_SOURCES})

    # Link the test executable against your library and GTest
    target_link_libraries(tests_smallkv PRIVATE smallkv_lib GTest::gtest GTest::gtest_main Threads::Threads)

    # Include the GTest header directory
    target_include_directories(tests_smallkv PRIVATE ${GTEST_INCLUDE_DIRS})

    # Add the test executable to CTest
    include(GoogleTest)
    gtest_discover_tests(tests_smallkv)

else()
    message(STATUS "Skipping unit tests")
endif()
# ---------------------------------