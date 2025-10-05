#include <scanner.hpp>

#include <iostream>

int main() {
    auto scanner = scanner::scanner{std::cin};

    while (true) {
        auto str_opt = scanner.next();
        if (!str_opt) {
            break;
        }
        else {
            auto token = str_opt.value();
            auto str = scanner.get_string(token);
            std::cout << token << ":";
            std::cout << '[';
            for (auto ch : str) {
                std::cout << ch;
            }
            std::cout << ']';
            std::cout << ' ';
        }
    }
    std::cout << std::endl;
}
