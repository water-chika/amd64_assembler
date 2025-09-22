#include "amd64_assembler.hpp"

#include <iostream>
#include <variant>
#include <vector>
#include <tuple>

using operands_t = std::variant<
        std::tuple<amd64::register_type::reg<32>, amd64::register_type::reg<32>>,
        std::tuple<amd64::register_type::reg<8>, uint8_t>,
        std::tuple<amd64::register_type::reg<16>, uint16_t>,
        std::tuple<amd64::register_type::reg<32>, uint32_t>
        >;

struct statement {
    amd64::operation operation;
    operands_t operands;
};

auto assemble(statement statement) {
    switch (statement.operation) {
    case amd64::operation::add:
    {
        return std::visit(
                [](auto operands) {
                    auto g_codes = amd64::add(std::get<0>(operands), std::get<1>(operands));
                    auto codes = std::vector<uint8_t>(g_codes.begin(), g_codes.end());
                    return codes;

                },
                statement.operands
            );
    }
    break;
    case amd64::operation::adc:
    {
        return std::visit(
                [](auto operands) {
                    auto g_codes = amd64::adc(std::get<0>(operands), std::get<1>(operands));
                    auto codes = std::vector<uint8_t>(g_codes.begin(), g_codes.end());
                    return codes;

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
                amd64::operation::adc,
                std::tuple<amd64::register_type::reg<32>, uint32_t>(amd64::ebx, uint32_t{7})
                },
            statement{
                amd64::operation::add,
                std::make_tuple(amd64::eax, amd64::ebx)
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
    }
    
    return 0;
}
