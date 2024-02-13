
cmake_minimum_required(VERSION 3.21)

project(
  CppUTestBasicDemo
  VERSION 1.0
  LANGUAGES CXX)

find_package(CppUTest REQUIRED)

set(SrcInc RAMFS/ unit_testing/fakes)
set(RunnerFile unit_testing/runner.cpp)
set(SrcFiles RAMFS/RamAccess.cpp RAMFS/RamFs.cpp RAMFS/RamFsFile.cpp RAMFS/RamFsFragment.cpp)
set(TestFiles unit_testing/test_groups/test.cpp )

add_executable(RAMFS_tests ${RunnerFile} ${TestFiles} ${SrcFiles})
add_compile_definitions(g)

target_link_libraries(RAMFS_tests PRIVATE
    CppUTest
    CppUTestExt)

target_include_directories(RAMFS_tests PRIVATE ${SrcInc})
