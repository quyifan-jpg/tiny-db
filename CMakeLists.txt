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

# --- Add this line ---
include(CTest)
# --------------------

# gtest
find_package(GTest REQUIRED)
find_package(Threads REQUIRED) # Needed for GTest and potentially your code

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
        src/sst_parser/*.cpp
        src/sst_parser/*.h
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
        src/sst_parser/*.cpp
        src/sst_parser/*.h
        src/table/*.cpp
        src/table/*.h
        src/utils/*.cpp
        src/utils/*.h
        src/wal/*.cpp
        src/wal/*.h)

# Create a library from the smallkv code
add_library(smallkv_lib ${SRC_WITHOUT_MAIN})
target_link_libraries(smallkv_lib PRIVATE pthread libabsl_crc32c.a libabsl_crc_internal.a libabsl_raw_logging_internal.a)

# Create the smallkv executable
add_executable(smallkv src/main.cpp)
target_link_libraries(smallkv PRIVATE smallkv_lib)

# Add debug information for GDB
set_target_properties(smallkv PROPERTIES
    CMAKE_BUILD_TYPE Debug
    COMPILE_FLAGS "-g3 -O0"
)

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