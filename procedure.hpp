#pragma once

namespace procedure {
    struct procedure {
        std::vector<uint32_t> reads;
        std::vector<uint32_t> writes;

        std::vector<instruction> instructions;
    };

    auto inline_procedures(std::vector<procedure> procedures, uint32_t start_procedure) {
        auto instructions = std::vector<instruction>{};
        const auto& procedure = procedures[start_procedure];
        uint32_t start_register = 0;

        uint32_t max_used_register = 0;

        std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>> call_stack{};

        auto procedure = start_procedure;
        uint32_t instruction_index = 0;
        while (true) {
            if (instruction_index == procedure.instructions.size()) {
                if (call_stack.empty()) {
                    break;
                }
                else {
                    auto [proc, i, r, m] = call_stack.back();call_stack.pop_back();
                    procedure = proc;
                    instruction_index = i;
                    start_register = r;
                    max_used_register = m;

                    const auto& instruction = procedure.instructions[instruction_index];
                    auto callee = get_calee(instruction);
                    auto writes = get_call_writes(instruction);
                    instructions.append_ranges(gen_copy_instructions(writes, procedures[callee].writes));

                    instruction_index++;
                    continue;
                }
            }
            const auto& instruction = procedure.instructions[instruction_index];
            if (is_call(instruction)) {
                auto callee = get_calee(instruction);
                auto reads = get_call_reads(instruction);
                auto writes = get_call_writes(instruction);
                instructions.append_ranges(gen_copy_instructions(procedures[callee].reads), reads);
                call_stack.emplace_back(procedure, instruction_index, start_register, max_used_register);
                procedure = callee;
                instruction_index = 0;
                start_register = max_used_register+1;
            }
            else {
                auto out_instruction = rename_registers(instruction, start_register);
                max_used_register = max(max_register(out_instruction),max_used_register);
                instructions.emplace_back(out_instruction);
                instruction_index++;
            }
        }

        return instructions;
    }
}
