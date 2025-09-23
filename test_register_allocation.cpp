#include "register_allocation.hpp"

#include <iostream>

template<typename Register, size_t Register_count = std::numeric_limits<Register>::max()>
struct instruction {
    using register_t = Register;

    static constexpr auto register_count = Register_count;

    std::vector<register_t> reads;
    std::vector<register_t> writes;

    auto& set_reads(std::vector<register_t> rs) {
        reads = std::move(rs);
        return *this;
    }
    auto& set_writes(std::vector<register_t> rs) {
        writes = std::move(rs);
        return *this;
    }
    auto& get_reads() const {
        return reads;
    }
    auto& get_writes() const {
        return writes;
    }
};

int main() {
    using namespace register_allocation;

    auto in_instructions = std::vector<instruction<uint32_t>>{
        {
            .writes= {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
            },
        },
        {
            .reads= {
                0, 1, 2, 3, 4, 5
            },
            .writes= {
                16, 17, 18
            }
        },
        {
            .reads= {
                16,17,18
            },
            .writes= {
                19,20,21
            }
        },
    };

    auto instructions = register_allocate<instruction<uint8_t, 17>>(in_instructions);

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
