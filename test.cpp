#include "amd64_assembler.hpp"

int main() {
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
    
    return 0;
}
