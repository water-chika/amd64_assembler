
add_executable(
    test_procedure
    test_procedure.cpp
)

target_link_libraries(test_procedure PUBLIC amd64_assembler)

add_executable(
    test_register_allocation
    test_register_allocation.cpp
)

target_link_libraries(test_register_allocation PUBLIC amd64_assembler)

add_executable(
    test_symbol
    test_symbol.cpp
)

target_link_libraries(test_symbol PUBLIC amd64_assembler)

add_executable(
    test
    test.cpp
)

target_link_libraries(test PUBLIC amd64_assembler)
