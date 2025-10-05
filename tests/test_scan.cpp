#include <scanner.hpp>

#include <iostream>
#include <fstream>

int test(std::istream& in) {
    auto scanner = scanner::scanner{in};

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
