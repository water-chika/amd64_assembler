#include <scanner.hpp>

#include <iostream>

int main() {
    auto scanner = scanner::word_splitter{std::cin};

    while (true) {
        auto str_opt = scanner.next();
        if (!str_opt) {
            break;
        }
        else {
            auto str = str_opt.value();
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
