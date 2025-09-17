#include "amd64_assembler.hpp"

#include <iostream>

int main() {
    using namespace amd64;
    using namespace amd64::register_type;
    auto codes = ret();
    for (auto code : codes) {
        std::cout << code;
    }
}
