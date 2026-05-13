cmake_minimum_required(VERSION 3.13)

project(coursework VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WARNING_FLAGS
  -Wall
  -Wextra
  -Wpedantic
  -Werror
)

add_library(huffman_core STATIC
  src/BitOperators.cpp
  src/BitOperators.h
  src/Encoder.cpp
  src/Encoder.h
  src/Decoder.cpp
  src/Decoder.h
  src/HuffmanTree.cpp
  src/HuffmanTree.h
  src/Queue.h
  src/Exceptions.h
)

target_include_directories(huffman_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(huffman_core PRIVATE ${WARNING_FLAGS})

add_executable(huffman
  src/main.cpp
)

target_link_libraries(huffman PRIVATE
  huffman_core
)

target_compile_options(huffman PRIVATE ${WARNING_FLAGS})

if(BUILD_TESTING)
  find_package(GTest QUIET)

  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(huffman_encoding_tests
    test/test.cpp
  )

  target_link_libraries(huffman_encoding_tests PRIVATE
    huffman_core
    GTest::gtest_main
  )

  target_compile_options(huffman_encoding_tests PRIVATE ${WARNING_FLAGS})

  target_compile_definitions(huffman_encoding_tests PRIVATE
    HUFFMAN_TEST_INPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/input"
    HUFFMAN_TEST_CODED_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/coded"
    HUFFMAN_TEST_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/output"
  )

  include(GoogleTest)
  gtest_discover_tests(huffman_encoding_tests)
endif()
