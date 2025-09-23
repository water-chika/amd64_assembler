#include "register_allocation.hpp"

#include <iostream>

using physical_register_t = uint8_t;

struct instruction {
    std::vector<physical_register_t> reads;
    std::vector<physical_register_t> writes;

    auto& set_read_physical_registers(std::vector<physical_register_t> prs) {
        reads = std::move(prs);
        return *this;
    }
    auto& set_write_physical_registers(std::vector<physical_register_t> prs) {
        writes = std::move(prs);
        return *this;
    }
};

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

    auto instructions = register_allocate<physical_register_t, 17, instruction>(in_instructions);

    for (const auto& instruction : instructions) {
        std::cout << "instruction:";
        for (const auto& pr : instruction.reads) {
            std::cout << (size_t)pr << ", ";
        }
        std::cout << "->";
        for (const auto& pr : instruction.writes) {
            std::cout << (size_t)pr << ", ";
        }
        std::cout << std::endl;
    }
    return 0;
}
