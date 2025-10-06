
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
    test_amd64_assembler
    test.cpp
)

target_link_libraries(test_amd64_assembler PUBLIC amd64_assembler)

add_test(
    NAME test_amd64_assembler
    COMMAND test_amd64_assembler
)

add_executable(
    test_scan
    test_scan.cpp
)

target_link_libraries(test_scan PUBLIC amd64_assembler)

add_test(
    NAME test_scan
    COMMAND test_scan ${CMAKE_CURRENT_SOURCE_DIR}/data/test_scan.s
)

