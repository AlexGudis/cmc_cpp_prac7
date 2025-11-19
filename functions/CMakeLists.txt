# mkdir build
# cd build
# cmake ..
# make

# ./main

cmake_minimum_required(VERSION 3.10)
project(FunctionLibrary)

set(CMAKE_CXX_STANDARD 11)

# Основная библиотека
add_library(func_lib func.cpp)

# Основная программа
add_executable(main main.cpp)
target_link_libraries(main func_lib)

# Тесты (если есть gtest)
find_package(GTest REQUIRED)
add_executable(test test.cpp)
target_link_libraries(test func_lib GTest::GTest GTest::Main)