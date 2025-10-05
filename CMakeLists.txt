cmake_minimum_required(VERSION 3.20)

project(amd64_assembler)

add_library(amd64_assembler
    amd64_assembler.cpp)

target_compile_features(amd64_assembler PUBLIC cxx_std_23)

target_include_directories(amd64_assembler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)

add_executable(
    output_binary
    output_binary.cpp
)

target_link_libraries(output_binary PUBLIC amd64_assembler)
