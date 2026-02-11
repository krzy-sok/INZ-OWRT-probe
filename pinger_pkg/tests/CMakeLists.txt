cmake_minimum_required(VERSION 3.16)
project(pinger_pkg)

include(CTest)
include(FetchContent)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)


FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# include(AddCatch2)

add_executable(tests.out
    test_host_file_reader.cpp
)

#                                      | here also hpp tested code
target_link_libraries(tests.out PRIVATE Catch2::Catch2WithMain)


include(CTest)
include(Catch)
catch_discover_tests(tests.out)

# add_test(NAME host_reader_test COMMAND tests.out)

#  -------- just ctest ---------

# target_compile_options(tests)
# enable_testing()
# add_executable(tests.out
#     test_host_file_reader.cpp
# )
# add_test(NAME host_reader_test COMMAND tests.out)