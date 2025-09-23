#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include <numeric>

namespace register_allocation {
    using memory_t = uint32_t;

    auto virtual_register_to_memory(auto reg) {
        return reg;
    }

    template<
        typename Out_instruction,
        typename In_instruction,
        typename Physical_register = Out_instruction::register_t,
        size_t Physical_register_count = Out_instruction::register_count,
        typename Virtual_register = In_instruction::register_t>
    std::vector<Out_instruction> register_allocate(std::vector<In_instruction> in_instructions, auto translate_registers, auto load_instruction, auto store_instruction) {
        auto out_instructions = std::vector<Out_instruction>{};
        //out_instructions.reserve(in_instructions.size());

        using physical_register_t = Physical_register;
        using virtual_register_t = Virtual_register;
        uint32_t current_physical_register = 0;
        auto physical_to_virtual_register = 
            std::array<virtual_register_t, Physical_register_count>{};
        std::ranges::iota(physical_to_virtual_register, 0);
        auto virtual_to_physical_register =
            std::unordered_map<virtual_register_t, physical_register_t>{};
        for (auto i : physical_to_virtual_register) {
            virtual_to_physical_register[i] = i;
        }
        auto physical_dirty =
            std::array<bool, Physical_register_count>{};

        auto insert_load_instruction =
            [&out_instructions, &load_instruction](physical_register_t pr, memory_t mem) {
                auto load = load_instruction(pr, mem);
                out_instructions.emplace_back(
                        load
                        );
            };
        auto insert_store_instruction =
            [&out_instructions, &store_instruction](memory_t mem, physical_register_t pr) {
                auto store = store_instruction(mem, pr);
                out_instructions.emplace_back(
                        store
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

            auto prev_vr = physical_to_virtual_register[pr];
            virtual_to_physical_register.erase(prev_vr);
            physical_to_virtual_register[pr] = vr;
            virtual_to_physical_register[vr] = pr;
        };

        auto next_physical_register =
            [&current_physical_register]
            () {
                auto reg = current_physical_register;
                current_physical_register = current_physical_register == Physical_register_count - 1 ? 0 : current_physical_register + 1;
                return reg;
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
            for (const auto& vr : instruction.get_reads()) {
                make_virtual_register_resident(vr, true);
                read_prs.emplace_back(virtual_to_physical_register[vr]);
            }
            auto write_prs = std::vector<physical_register_t>();
            for (const auto& vr : instruction.get_writes()) {
                make_virtual_register_resident(vr, false);
                assert(virtual_to_physical_register.contains(vr));
                assert(physical_to_virtual_register[virtual_to_physical_register[vr]] == vr);
                auto pr = virtual_to_physical_register[vr];
                write_prs.emplace_back(pr);
                physical_dirty[pr] = true;
            }

            out_instructions.emplace_back(
                    translate_registers(instruction, write_prs, read_prs)
                    );
        }
        return out_instructions;
    }
}
