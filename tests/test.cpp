#include "amd64_assembler.hpp"

#include <iostream>
#include <variant>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <unordered_map>

namespace use_virtual_register {
    using namespace amd64;
    struct virtual_register {
        size_t size;
        uint32_t index;

        auto operator<=>(const virtual_register& rhs) const& = default;
    };
}
    template<>
    struct std::hash<use_virtual_register::virtual_register>
    {
        std::size_t operator()(const use_virtual_register::virtual_register& reg) const noexcept {
            return reg.size | reg.index << 6;
        }
    };
namespace use_virtual_register {
    using operands_t = std::variant<
            std::tuple<virtual_register>,
            std::tuple<virtual_register, virtual_register>,
            std::tuple<virtual_register, virtual_register, virtual_register>
        >;

    void for_each_virtual_register(auto fun, operands_t operands) {
        std::visit(
                cpp_helper::overloads{
                    [&fun](std::tuple<virtual_register> operands) {
                        fun(std::get<0>(operands));
                    },
                    [&fun](std::tuple<virtual_register, virtual_register> operands) {
                        fun(std::get<0>(operands));
                        fun(std::get<1>(operands));
                    },
                    [&fun](std::tuple<virtual_register, virtual_register, virtual_register> operands) {
                        fun(std::get<0>(operands));
                        fun(std::get<1>(operands));
                        fun(std::get<2>(operands));
                    }
                },
                operands);
    }
    struct statement {
        operation op;
        operands_t operands;
    };

    struct assemble_context {
        std::array<std::optional<virtual_register>, 16> physical_to_virtual;
        std::unordered_set<virtual_register> allocated_physical_virtual_registers;
        std::unordered_map<virtual_register, register_type::reg<32>> virtual_to_physical;
    };

    amd64::operands_t to_physical_registers(const assemble_context& context , operands_t operands) {
        return std::visit(
                cpp_helper::overloads{
                    [&context](std::tuple<virtual_register> operands) {
                        return amd64::operands_t{
                                std::tuple<register_type::reg<32>>{context.virtual_to_physical.at(std::get<0>(operands))}
                                };
                    },
                    [&context](std::tuple<virtual_register, virtual_register> operands) {
                        return amd64::operands_t{
                                std::tuple{
                                    context.virtual_to_physical.at(std::get<0>(operands)),
                                    context.virtual_to_physical.at(std::get<1>(operands))
                                    }
                                };
                    },
                    [&context](std::tuple<virtual_register, virtual_register, virtual_register> operands) {
                        return amd64::operands_t{
                                std::tuple{
                                    context.virtual_to_physical.at(std::get<0>(operands)),
                                    context.virtual_to_physical.at(std::get<1>(operands)),
                                    context.virtual_to_physical.at(std::get<2>(operands))
                                    }
                                };
                    }
                },
                operands);
    }

    auto assemble(assemble_context& context, const statement& statement) {
        auto needed_registers = std::vector<virtual_register>{};
        auto need_allocated_registers = std::vector<virtual_register>{};
        for_each_virtual_register(
                [&needed_registers,
                 &need_allocated_registers,
                 &allocated=context.allocated_physical_virtual_registers](auto reg) {
                        needed_registers.emplace_back(reg);
                        if (!allocated.contains(reg)) {
                            need_allocated_registers.emplace_back(reg);
                        }
                },
                statement.operands);
        auto codes = std::vector<uint8_t>{};
        while (!need_allocated_registers.empty()) {
            int i = rand() % context.physical_to_virtual.size();
            if (context.physical_to_virtual[i] &&
                    std::ranges::contains(needed_registers, context.physical_to_virtual[i].value())) {
                auto virtual_reg = context.physical_to_virtual[i].value();
                {
                auto g_codes = mov(rsp, register_type::reg<64>{static_cast<register_type::reg64>(i)}); // correct it
                codes.append_range(g_codes);
                }
                {
                auto g_codes = mov(register_type::reg<64>{static_cast<register_type::reg64>(i)}, rsp); // correct it
                codes.append_range(g_codes);
                }
                context.physical_to_virtual[i] = need_allocated_registers.back();
                context.allocated_physical_virtual_registers.erase(virtual_reg);
                context.allocated_physical_virtual_registers.emplace(need_allocated_registers.back());

                need_allocated_registers.pop_back();
            }
        }

        auto g_codes = amd64::assemble(
                amd64::statement{
                    statement.op, 
                    to_physical_registers(context, statement.operands)
                }
                );
    }
}

int main() {
    try{
        using namespace amd64;
        using namespace amd64::register_type;

        auto t = adc(al, 5);
        assert(t.size() == 2 && t[0] == 0x14 && t[1] == 0x5);

        assert(
                std::ranges::equal(
                adc(ebx, 5),
                std::array<uint8_t, 6>{ 0x81, 0xd3, 5, 0, 0, 0})
              );

        assert(
                std::ranges::equal(
                    adc(eax, ebx),
                    std::array<uint8_t, 2>{0x11, 0xd8}
                    )
              );

        assert(std::ranges::equal(
                    ret(),
                    std::array<uint8_t, 1>{0xc3}
                    )
                );

        auto codes = assemble(
            std::vector{
            statement{
                operation::adc,
                std::tuple<register_type::reg<32>, uint32_t>(ebx, uint32_t{7})
                },
            statement{
                operation::add,
                std::make_tuple(eax, ebx)
                }
            }
        );
        assert(
                std::ranges::equal(
                    codes,
                    std::vector<uint8_t>{ 0x81, 0xd3, 7, 0, 0, 0, 0x01, 0xd8 }
                )
              );

        add(1, eax);
    }
    catch (std::exception& except) {
        std::cout << except.what() << std::endl;
        return -1;
    }
    
    return 0;
}
