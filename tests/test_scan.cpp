#include <scanner.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <span>
#include <algorithm>
#include <functional>

#include <amd64_assembler.hpp>

std::vector<char> operator ""_v (const char* t, size_t n) {
    std::vector<char> str(n);
    std::ranges::copy(
            std::span{t, n},
            str.begin());
    return str;
}

void print_token(scanner::scanner& scanner, size_t token) {
    auto str = scanner.get_string(token);
    std::cout << token << ":";
    std::cout << '[';
    for (auto ch : str) {
        std::cout << ch;
    }
    std::cout << ']';
    std::cout << ' ';
}

void print_codes(auto codes) {
    std::cout << std::hex << std::showbase;
    for (auto code : codes) {
        std::cout << static_cast<uint32_t>(code) << ' ';
    }
}

template<typename Int = int32_t>
Int to_integer(const std::vector<char>& str) {
    Int integer = 0;
    for (auto ch : str) {
        auto digit = ch - '0';
        integer *= 10;
        integer += digit;
    }

    return integer;
}

int test(std::istream& in) {
    auto scanner = scanner::scanner{in};

    auto add_token = scanner.add_string("add"_v);
    auto ret_token = scanner.add_string("ret"_v);

    auto ax_token = scanner.add_string("ax"_v);
    auto rax_token = scanner.add_string("rax"_v);
    auto rbx_token = scanner.add_string("rbx"_v);

    auto register_begin_token = ax_token;
    auto register_end_token = rbx_token;

    using reg_variant = 
        std::variant<
            amd64::register_type::ax_r<8>,
            amd64::register_type::ax_r<16>,
            amd64::register_type::ax_r<32>,
            amd64::register_type::ax_r<64>,
            amd64::register_type::reg<8>,
            amd64::register_type::reg<16>,
            amd64::register_type::reg<32>,
            amd64::register_type::reg<64>
            >;
    auto token_to_register = std::unordered_map<
        size_t,
        reg_variant
        >{
            {ax_token, amd64::ax},
        };

    auto add = [](reg_variant reg_var, auto imm) {
        return std::visit(
                [&imm](auto reg) {
                    auto codes_array = amd64::add(reg, imm);
                    auto codes = std::vector<uint8_t>(codes_array.size());
                    std::ranges::copy(codes_array, codes.begin());
                    return codes;
                },
                reg_var
                );
    };

    auto token_func_map = std::unordered_map<size_t, std::function<void(size_t)>>{};
    token_func_map.emplace(add_token,
            [&scanner,
             register_begin_token,
             register_end_token,
             &token_to_register,
             &add
            ](size_t token) {
                auto dst = scanner.next().value();
                auto src = scanner.next().value();

                print_token(scanner, token);
                print_token(scanner, dst);
                print_token(scanner, src);

                std::cout << "\t\t\t|\t";

                if (dst >= register_begin_token && dst <= register_end_token) {
                    auto& src_str = scanner.get_string(src);
                    if (isdigit(src_str[0])) {
                        auto imm = to_integer(src_str);
                        print_codes(add(token_to_register[dst], imm));
                    }
                }

                std::cout << std::endl;
            }
            );
    token_func_map.emplace(ret_token,
            [&scanner
            ](size_t token) {
                print_token(scanner, token);

                std::cout << "\t\t\t|\t";

                print_codes(amd64::ret());

                std::cout << std::endl;
            }
            );

    while (true) {
        auto str_opt = scanner.next();
        if (!str_opt) {
            break;
        }
        else {
            auto token = str_opt.value();
            auto func = token_func_map[token];
            func(token);
        }
    }
    std::cout << std::endl;
    return 0;
}

int main(int argc, const char* argv[]) {
    try {
        if (argc <= 1) {
            return test(std::cin);
        }
        else {
            auto in = std::ifstream{argv[1]};
            return test(in);
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}
