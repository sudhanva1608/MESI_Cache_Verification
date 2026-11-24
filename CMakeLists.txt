
cmake_minimum_required(VERSION 3.20)

project(MESI_Cache_Verification)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

enable_testing()

add_library(mesi_coherence
    src/coherence/mesi_state.cpp
)

target_include_directories(mesi_coherence PUBLIC
    src/coherence
)

add_executable(mesi_tests
    tests/unit/mesi_state_test.cpp
    tests/unit/cache_test.cpp
    tests/unit/main_memory_test.cpp
)

target_link_libraries(mesi_tests
    PRIVATE
    mesi_coherence
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mesi_tests)

