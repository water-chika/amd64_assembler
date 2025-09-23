#include "register_allocation.hpp"

#include <iostream>

template<typename Register, size_t Register_count = std::numeric_limits<Register>::max()>
struct instruction {
    using register_t = Register;

    static constexpr auto register_count = Register_count;

    uint32_t op;
    std::vector<register_t> reads;
    std::vector<register_t> writes;

    std::vector<uint32_t> read_memories;
    std::vector<uint32_t> write_memories;

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

    auto& set_read_memories(std::vector<uint32_t> memories) {
        read_memories = memories;
        return *this;
    }
    auto& set_write_memories(std::vector<uint32_t> memories) {
        write_memories = memories;
        return *this;
    }
};

int main() {
    using namespace register_allocation;

    auto in_instructions = std::vector<instruction<uint32_t>>{
        {
            .op = 8,
            .writes= {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
            },
        },
        {
            .op = 9,
            .reads= {
                0, 1, 2, 3, 4, 5
            },
            .writes= {
                16, 17, 18
            }
        },
        {
            .op =10,
            .reads= {
                16,17,18
            },
            .writes= {
                19,20,21
            }
        },
        {
            .op = 10,
            .reads = {
                0, 1, 21,
            },
            .writes = {
                22, 23, 24
            }
        }
    };

    auto instructions = register_allocate<instruction<uint8_t, 17>>(in_instructions,
            [](auto in_instruction, auto writes, auto reads) {
                return instruction<uint8_t, 17>{in_instruction.op}
                    .set_reads(reads).set_writes(writes);
            },
            [](auto pr, auto mem) {
                return instruction<uint8_t, 17>{0}.set_writes({pr}).set_read_memories({mem});
            },
            [](auto mem, auto pr) {
                return instruction<uint8_t, 17>{1}.set_reads({pr}).set_write_memories({mem});
            }
            );

    for (const auto& instruction : instructions) {
        std::cout << instruction.op << ":";
        for (const auto& pr : instruction.reads) {
            std::cout << (size_t)pr << ", ";
        }
        for (const auto& mem : instruction.read_memories) {
            std::cout << "[" << mem << "]" << ", ";
        }
        std::cout << "->";
        for (const auto& pr : instruction.writes) {
            std::cout << (size_t)pr << ", ";
        }
        for (const auto& mem : instruction.write_memories) {
            std::cout << "[" << mem << "]" << ", ";
        }
        std::cout << std::endl;
    }
    return 0;
}
