#pragma once

#include "instruction.hpp"

namespace amd64 {
    constexpr auto adc      = arithmetic_instruction<0x15, {0x81,2}, 0x11>{};
    constexpr auto add      = integrate_instructions{
        arithmetic_instruction<0x05, {0x81,0}, 0x01>{},
        wrong_operands{}
    };
    constexpr auto bit_and  = arithmetic_instruction<0x25, {0x81,4}, 0x21>{};
    constexpr auto cmp      = arithmetic_instruction<0x3d, {0x81,7}, 0x39>{};
    constexpr auto bit_or   = arithmetic_instruction<0x0d, {0x81,1}, 0x09>{};
    constexpr auto sbb      = arithmetic_instruction<0x1d, {0x81,3}, 0x19>{};
    constexpr auto sub      = arithmetic_instruction<0x2d, {0x81,5}, 0x29>{};
    constexpr auto test     = arithmetic_instruction<0xa9, {0xf7,0}, 0x85>{};
    constexpr auto bit_xor  = arithmetic_instruction<0x35, {0x81,6}, 0x31>{};

    constexpr auto cwb  = opcode_instruction<0x98, 16>{};
    constexpr auto cwde = opcode_instruction<0x98, 32>{};
    constexpr auto cdqe = opcode_instruction<0x98, 64>{};

    constexpr auto cwd = opcode_instruction<0x99, 16>{};
    constexpr auto cdq = opcode_instruction<0x99, 32>{};
    constexpr auto cqo = opcode_instruction<0x99, 64>{};

    constexpr auto clc = opcode_instruction<0xf8>{};
    constexpr auto cld = opcode_instruction<0xfc>{};

    constexpr auto clzero = opcode_instruction<std::to_array({0x0f, 0x01, 0xfc})>{};

    constexpr auto cmc = opcode_instruction<0xf5>{};

    constexpr auto cpuid = opcode_instruction<std::to_array({0x0f, 0xa2})>{};

    constexpr auto nop = opcode_instruction<0x90>{};

    constexpr auto ret = opcode_instruction<0xc3>{};

    constexpr auto bit_not = regmem_instruction<{0xf7, 2}>{};
    constexpr auto neg  = regmem_instruction<{0xf7, 3}>{};
    constexpr auto mul  = regmem_instruction<{0xf7, 4}>{};
    constexpr auto imul = regmem_instruction<{0xf7, 5}>{};
    constexpr auto div  = regmem_instruction<{0xf7, 6}>{};
    constexpr auto idiv = regmem_instruction<{0xf7, 7}>{};

    template<bitset<4> Condition_code>
    using jcc_instruction = imm_instruction<bit4{7} + Condition_code, bit8{0x0f} + bit4{8} + Condition_code>;

    template<bitset<4> Condition_code>
    constexpr auto jcc = jcc_instruction<Condition_code>{};

    constexpr auto jo  = jcc_instruction<0x0>{};
    constexpr auto jno = jcc_instruction<0x1>{};
    constexpr auto jb  = jcc_instruction<0x2>{};
    constexpr auto jnb = jcc_instruction<0x3>{};
    constexpr auto jz  = jcc_instruction<0x4>{};
    constexpr auto jnz = jcc_instruction<0x5>{};
    constexpr auto jbe = jcc_instruction<0x6>{};
    constexpr auto jnbe= jcc_instruction<0x7>{};
    constexpr auto js  = jcc_instruction<0x8>{};
    constexpr auto jns = jcc_instruction<0x9>{};
    constexpr auto jp  = jcc_instruction<0xa>{};
    constexpr auto jnp = jcc_instruction<0xb>{};
    constexpr auto jl  = jcc_instruction<0xc>{};
    constexpr auto jnl = jcc_instruction<0xd>{};
    constexpr auto jle = jcc_instruction<0xe>{};
    constexpr auto jnle= jcc_instruction<0xf>{};

    constexpr auto jmp = cpp_helper::overloads<
            imm_instruction<0xeb, 0xe9>,
            regmem_instruction<{0xff, 4}, false>
        >{};

    constexpr auto lea = reg_mem_instruction<0x8d>{};

    constexpr
    auto mov = cpp_helper::overloads<
        regmem_reg_instruction<0x89>,
        reg_reg_instruction<0x89>,
        reg_regmem_instruction<0x8b>,
        ax_imm_instruction<0xa1>,
        imm_ax_instruction<0xa3>,
        reg_imm_instruction<0xb0, 0xb8>,
        regmem_imm_instruction<{0xc7,0}>
    >{};

}

#include <vector>
namespace amd64 {
    using operands_t = std::variant<
            std::tuple<register_type::reg<32>>,
            std::tuple<register_type::reg<32>, register_type::reg<32>>,
            std::tuple<register_type::reg<8>, uint8_t>,
            std::tuple<register_type::reg<16>, uint16_t>,
            std::tuple<register_type::reg<32>, uint32_t>,
            std::tuple<register_type::reg<32>, register_type::reg<32>, register_type::reg<32>>
            >;

    template<typename T>
    concept operands_2 = std::tuple_size_v<T> == 2;

    struct statement {
        operation op;
        operands_t operands;
    };

    auto assemble(statement statement) {
        switch (statement.op) {
        case operation::add:
        {
            return std::visit(
                    cpp_helper::overloads{
                        [](operands_2 auto operands) {
                            auto g_codes = add(std::get<0>(operands), std::get<1>(operands));
                            auto codes = std::vector<uint8_t>(g_codes.begin(), g_codes.end());
                            return codes;
                        },
                        [](auto operands) {
                            //throw std::runtime_error{"operands count error"};
                            return std::vector<uint8_t>{};
                        }
                    },
                    statement.operands
                );
        }
        break;
        case operation::adc:
        {
            return std::visit(
                    cpp_helper::overloads{
                        [](operands_2 auto operands) {
                            auto g_codes = adc(std::get<0>(operands), std::get<1>(operands));
                            auto codes = std::vector<uint8_t>(g_codes.begin(), g_codes.end());
                            return codes;
                        },
                        [](auto operands) {
                            throw std::runtime_error{"operands count error"};
                            return std::vector<uint8_t>{};
                        }
                    },
                    statement.operands
                );
        }
        break;
        default:
        {
            throw std::runtime_error{"unknown operation"};
            return std::vector<uint8_t>{};
        }
        break;
        }
    }

    auto assemble(std::vector<statement> statements) {
        auto codes = std::vector<uint8_t>{};
        for (auto& statement : statements) {
            auto g_codes = assemble(statement);
            codes.append_range(g_codes);
        }
        return codes;
    }
}
