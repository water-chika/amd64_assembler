#include "register_allocation.hpp"

#include <iostream>

std::ostream& operator<<(std::ostream& out, register_allocation::physical_register_t pr) {
    return out << pr.index;
}

int main() {
    using namespace register_allocation;

    auto in_instructions = std::vector<in_instruction>{
        in_instruction{
            .write_virtual_registers = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
            },
        },
        in_instruction{
            .read_virtual_registers = {
                0, 1, 2, 3, 4, 5
            },
            .write_virtual_registers = {
                16, 17, 18
            }
        },
        in_instruction{
            .read_virtual_registers = {
                16,17,18
            },
            .write_virtual_registers = {
                19,20,21
            }
        },
    };

    auto instructions = register_allocate(in_instructions);

    for (const auto& instruction : instructions) {
        std::cout << "instruction:";
        for (const auto& pr : instruction.read_physical_registers) {
            std::cout << pr << ", ";
        }
        std::cout << "->";
        for (const auto& pr : instruction.write_physical_registers) {
            std::cout << pr << ", ";
        }
        std::cout << std::endl;
    }
    return 0;
}
