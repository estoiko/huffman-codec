cmake_minimum_required(VERSION 3.16)

project(HuffmanArchiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(huff
  src/main.cpp
  src/Encoder.cpp
  src/Decoder.cpp
  src/HuffmanTree.cpp
  src/BitOperators.cpp
)

target_include_directories(huff PRIVATE include)

add_executable(tests
  test/tests.cpp
  src/Encoder.cpp
  src/Decoder.cpp
  src/HuffmanTree.cpp
  src/BitOperators.cpp
)

target_include_directories(tests PRIVATE include)
