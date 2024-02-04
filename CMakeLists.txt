
cmake_minimum_required(VERSION 3.21)

project(
  CppUTestBasicDemo
  VERSION 1.0
  LANGUAGES CXX)

find_package(CppUTest REQUIRED)

set(SrcInc RAMFS/)
set(RunnerFile tests/runner.cpp)
set(SrcFiles RAMFS/RAMFS.cpp )
set(TestFiles tests/test.cpp)

add_executable(RAMFS_tests ${RunnerFile} ${TestFiles} ${SrcFiles})

target_link_libraries(RAMFS_tests PRIVATE
    CppUTest
    CppUTestExt)

target_include_directories(RAMFS_tests PRIVATE ${SrcInc})
