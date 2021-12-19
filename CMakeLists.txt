cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

# UNX
add_library(LibUnx STATIC src/unx.cpp)
target_include_directories(LibUnx PUBLIC include)

# UNX usage example
project(UnxExample)

add_executable(
        UnxExample
        src/example/forager.cpp
)

TARGET_LINK_LIBRARIES(UnxExample LibUnx)