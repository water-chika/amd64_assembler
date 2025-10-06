#include <scanner.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <span>
#include <algorithm>

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
    for (auto code : codes) {
        std::cout << static_cast<uint32_t>(code) << ' ';
    }
}

int test(std::istream& in) {
    auto scanner = scanner::scanner{in};

    auto add_token = scanner.add_string("add"_v);
    auto ret_token = scanner.add_string("ret"_v);

    auto ax_token = scanner.add_string("ax"_v);
    auto rax_token = scanner.add_string("rax"_v);
    auto rbx_token = scanner.add_string("rbx"_v);

    while (true) {
        auto str_opt = scanner.next();
        if (!str_opt) {
            break;
        }
        else {
            auto token = str_opt.value();
            if (token == add_token) {
                auto dst = scanner.next().value();
                auto src = scanner.next().value();

                print_token(scanner, token);
                print_token(scanner, dst);
                print_token(scanner, src);

                print_codes(amd64::add(dst, src));

                std::cout << std::endl;
            }
            else if (token == ret_token) {
                print_token(scanner, token);
                std::cout << std::endl;

                amd64::ret();
            }
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
