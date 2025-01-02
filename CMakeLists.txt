cmake_minimum_required(VERSION 3.14)
project(text_duplicate_finder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add library
add_library(text_processing
        src/text_processing/utf8_handler.cpp
        include/text_processing/suffix_array_builder.hpp
        include/text_processing/naive_suffix_builder.hpp
        include/data/document_store.hpp
        src/text_processing/naive_suffix_builder.cpp
        src/text_processing/suffix_array_builder.cpp
)

# Fetch and configure Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add test executables
add_executable(utf8_tests
        tests/unit/text_processing/test_utf8_handler.cpp
)

# Add test executables
add_executable(naive_suffix_builder_tests
        tests/unit/text_processing/test_naive_suffix_builder.cpp
)

# Add test executables
add_executable(document_store
        tests/unit/data/test_document_store.cpp
)

# Link libraries
target_link_libraries(utf8_tests
        PRIVATE
        text_processing
        GTest::gtest_main
)

target_link_libraries(document_store
        PRIVATE
        text_processing
        GTest::gtest_main
)

target_link_libraries(naive_suffix_builder_tests
        PRIVATE
        text_processing
        GTest::gtest_main
        GTest::gmock_main
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(utf8_tests)
gtest_discover_tests(naive_suffix_builder_tests)
gtest_discover_tests(document_store)