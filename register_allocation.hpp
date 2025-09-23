#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace register_allocation {
    using virtual_register_t = uint32_t;
    struct physical_register {
        uint32_t index : 4;
        operator uint32_t() {
            return index;
        }
    };
    using physical_register_t = physical_register;
    constexpr auto max_physical_register = physical_register{ 0x0f };
    constexpr auto physical_register_count = 0x10;

    using memory_t = uint32_t;

    struct in_instruction {
        std::vector<virtual_register_t> read_virtual_registers;
        std::vector<virtual_register_t> write_virtual_registers;
    };

    struct out_instruction {
        std::vector<physical_register_t> read_physical_registers;
        std::vector<physical_register_t> write_physical_registers;
    };

    auto virtual_register_to_memory(virtual_register_t reg) {
        return reg;
    }

    std::vector<out_instruction> register_allocate(std::vector<in_instruction> in_instructions) {
        auto out_instructions = std::vector<out_instruction>{};
        //out_instructions.reserve(in_instructions.size());

        uint32_t current_physical_register = 0;
        std::vector<virtual_register_t> physical_to_virtual_register(physical_register_count);
        std::unordered_map<virtual_register_t, physical_register_t> virtual_to_physical_register{};
        std::vector<bool> physical_dirty(physical_register_count);

        auto insert_load_instruction =
            [&out_instructions](physical_register_t pr, memory_t mem) {
                out_instructions.emplace_back(
                        out_instruction{
                        .write_physical_registers = {pr},
                        }
                        );
            };
        auto insert_store_instruction =
            [&out_instructions](memory_t mem, physical_register_t pr) {
                out_instructions.emplace_back(
                        out_instruction{
                        .read_physical_registers = {pr},
                        }
                        );
        };

        auto spill_physical_register =
            [
            &physical_dirty,
            &physical_to_virtual_register,
            &insert_store_instruction
            ](physical_register_t reg) {
            if (physical_dirty[reg]) {
                const auto& physical_register_memory = virtual_register_to_memory(physical_to_virtual_register[reg]);
                insert_store_instruction(physical_register_memory, reg);
                physical_dirty[reg] = false;
            }
        };
        auto assign_physical_virtual_register =
            [
            &physical_to_virtual_register,
            &virtual_to_physical_register,
            &spill_physical_register,
            &insert_load_instruction
            ]
         (physical_register_t pr, virtual_register_t vr, bool is_read) {
            spill_physical_register(pr);
            if (is_read) {
                const auto& vr_memory = virtual_register_to_memory(vr);
                insert_load_instruction(pr, vr_memory);
            }
            physical_to_virtual_register[pr] = vr;
            virtual_to_physical_register[vr] = pr;
        };

        auto next_physical_register =
            [&current_physical_register]
            () {
                auto reg = current_physical_register;
                current_physical_register = current_physical_register == max_physical_register.index ? 0 : current_physical_register + 1;
                return physical_register{reg};
        };

        auto make_virtual_register_resident =
            [&virtual_to_physical_register,
             &assign_physical_virtual_register,
             &next_physical_register
            ](virtual_register_t vr, bool is_read) {
                if (!virtual_to_physical_register.contains(vr)) {
                    assign_physical_virtual_register(next_physical_register(), vr, is_read);
                }
        };

        for (const auto& instruction : in_instructions) {
            auto read_prs = std::vector<physical_register_t>();
            for (const auto& vr : instruction.read_virtual_registers) {
                make_virtual_register_resident(vr, true);
                read_prs.emplace_back(virtual_to_physical_register[vr]);
            }
            auto write_prs = std::vector<physical_register_t>();
            for (const auto& vr : instruction.write_virtual_registers) {
                make_virtual_register_resident(vr, false);
                auto pr = virtual_to_physical_register[vr];
                write_prs.emplace_back(pr);
                physical_dirty[pr] = true;
            }

            out_instructions.emplace_back(
                    out_instruction{
                    .read_physical_registers = read_prs,
                    .write_physical_registers = write_prs
                    }
                    );
        }
        return out_instructions;
    }
}
