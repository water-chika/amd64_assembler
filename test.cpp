#include "amd64_assembler.hpp"

int main() {
    auto t = amd64::adc(amd64::register_type::al{}, 5);
    assert(t.size() == 2 && t[0] == 0x14 && t[1] == 0x5);
    return 0;
}
