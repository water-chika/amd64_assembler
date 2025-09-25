#pragma once

#include <vector>
#include <cstdint>

namespace procedure {
    template<typename Instruction>
    struct procedure {
        std::vector<uint32_t> reads;
        std::vector<uint32_t> writes;

        std::vector<Instruction> instructions;
    };

    template<typename Instruction>
    auto inline_procedures(std::vector<procedure<Instruction>> procedures, uint32_t start_procedure_index,
            auto get_callee,
            auto gen_copy_instructions,
            auto is_call,
            auto get_call_reads,
            auto get_call_writes,
            auto rename_registers,
            auto next_register
            ) {
        auto instructions = std::vector<Instruction>{};

        auto procedure_index = start_procedure_index;
        uint32_t instruction_index = 0;
        uint32_t start_register = 0;
        uint32_t next_start_register = 0;

        std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> call_stack{};

        while (true) {
            const auto& procedure = procedures[procedure_index];
            if (instruction_index == procedure.instructions.size()) {
                if (call_stack.empty()) {
                    break;
                }
                else {
                    auto callee_writes = rename_registers(procedure.writes, start_register);
                    auto [proc, i, r, m] = call_stack.back();call_stack.pop_back();
                    procedure_index = proc;
                    instruction_index = i;
                    start_register = r;
                    next_start_register = m;

                    const auto& instruction = procedures[procedure_index].instructions[instruction_index];
                    auto callee = get_callee(instruction);
                    auto writes = get_call_writes(instruction);
                    instructions.append_range(gen_copy_instructions(writes, callee_writes));

                    instruction_index++;
                    continue;
                }
            }
            const auto& instruction = procedure.instructions[instruction_index];
            if (is_call(instruction)) {
                auto callee = get_callee(instruction);
                auto reads = get_call_reads(instruction);
                auto writes = get_call_writes(instruction);
                instructions.append_range(gen_copy_instructions(rename_registers(procedures[callee].reads, next_start_register), reads));
                call_stack.emplace_back(procedure_index, instruction_index, start_register, next_start_register);
                procedure_index = callee;
                instruction_index = 0;
                start_register = next_start_register;
            }
            else {
                auto out_instruction = rename_registers(instruction, start_register);
                next_start_register = std::max(next_register(out_instruction),next_start_register);
                instructions.emplace_back(out_instruction);
                instruction_index++;
            }
        }

        return instructions;
    }
}
