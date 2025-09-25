#include "procedure.hpp"

#include "cpp_helper/cpp_helper.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

struct instruction{
    uint32_t op;
    std::vector<uint32_t> reads;
    std::vector<uint32_t> writes;

    std::vector<uint32_t> read_memories;
    std::vector<uint32_t> write_memories;

    uint32_t callee;

    auto& get_writes() const {
        return writes;
    }
};


int main() {
    using procedure::procedure;
    auto procedures = std::vector<procedure<instruction>>{
        {
            {},
            {},
            {
                {1, {}, {0, 1}, {}, {}},
                {3, {0, 1}, {2}}
            }
        },
        {
            {},
            {},
            {
                {1, {}, {0, 1}},
                {1, {0, 1}, {2, 3}},
                {0, {}, {}, {}, {}, 0},
                {1, {2, 3}, {4, 5}},
                {2, {0, 3}, {6, 7}}
            }
        }
    };

    auto instructions = procedure::inline_procedures(procedures, 1,
            [](auto instruction) {
                return instruction.callee;
            },
            [](std::vector<uint32_t> dests, std::vector<uint32_t> srcs) {
                assert(dests.size() == srcs.size());
                if (srcs.size() == 0) {
                    return std::vector<instruction>{};
                }
                return std::vector{instruction{1, srcs, dests}};
            },
            [](auto instruction) { return instruction.op == 0; },
            [](auto instruction) { return instruction.reads; },
            [](auto instruction) { return instruction.writes; },
            cpp_helper::overloads{
                [](procedure<instruction> proc, auto start_register) {
                    for (auto& instruction : proc.instructions) {
                        for (auto& read : instruction.reads) {
                            read += start_register;
                        }
                        for (auto& write : instruction.writes) {
                            write += start_register;
                        }
                    }
                    return proc;
                },
                [](instruction instruction, auto start_register) {
                    for (auto& read : instruction.reads) {
                        read += start_register;
                    }
                    for (auto& write : instruction.writes) {
                        write += start_register;
                    }
                    return instruction;
                },
                [](std::vector<uint32_t> regs, auto  start_register) {
                    for (auto& reg : regs) {
                        reg += start_register;
                    }
                    return regs;
                }
            },
            [](auto instruction) {
                auto max = [](auto range) {
                    return std::ranges::fold_left(range, 0u,
                            [](auto l, auto r){
                                return std::max(l, r)+1;
                            });
                };
                return std::max(max(instruction.reads), max(instruction.writes));
            }
            );

    for (auto inst : instructions) {
        std::cout << inst.op << ": " ;
        for (auto read : inst.reads) {
            std::cout << read << ", " ;
        }
        for (auto read : inst.read_memories) {
            std::cout << "[" << read << "]" << ", " ;
        }
        std::cout << "->";
        for (auto write : inst.writes) {
            std::cout << write << ", ";
        }
        for (auto write : inst.write_memories) {
            std::cout << "[" << write << "]" << ", " ;
        }

        std::cout << std::endl;
    }

    return 0;
}
