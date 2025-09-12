#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cassert>

namespace amd64 {
    constexpr auto instruction_length_limit = 15;

    namespace register_type {
        struct al{};
        struct ax{};
        struct eax{};
        struct rax{};

        enum class reg8 : uint8_t {
            al = 0,
            cl = 1,
            dl = 2,
            bl = 3,
            ah = 4,
            ch = 5,
            dh = 6,
            bh = 7
        };
        struct mem8 {
        };
    }

    template<size_t N>
    class bits {
    public:
        static_assert(N <= 8);
        bits() = default;
        bits(uint8_t v) : m_v{v}
        {
            assert(v < (1<<N));
        }
        operator uint8_t() const {
            return m_v;
        }
    private:
        uint8_t m_v : N;
    };

    using bit1 = bits<1>;
    using bit2 = bits<2>;
    using bit3 = bits<3>;

    class modrm {
    public:
        modrm() = default;
        modrm(bit2 mod, bit3 reg, bit3 rm) : m_mod{mod}, m_reg{reg}, m_rm{rm}{
        }

        operator uint8_t() const {
            return (m_mod << 6) | (m_reg << 3) | m_rm;
        }

        auto mod() const {
            return m_mod;
        }
        auto reg() const {
            return m_reg;
        }
        auto rm() const {
            return m_rm;
        }

        auto set_reg(bit3 reg) && {
            return modrm{m_mod, reg, m_rm};
        }
        auto set_rm(register_type::reg8 dst) && {
            return modrm{3, m_reg, static_cast<uint8_t>(dst)};
        }
    private:
        bit3 m_rm;
        bit3 m_reg;
        bit2 m_mod;
    };

    class sib {
    public:
        sib(bit2 scale, bit3 index, bit3 base) : m_scale{scale}, m_index{index}, m_base{base} {
        }

        operator uint8_t() const {
            return (m_scale << 6) | (m_index << 3) | m_base;
        }

        auto scale() const {
            return m_scale;
        }
        auto index() const {
            return m_index;
        }
        auto base() const {
            return m_base;
        }
    private:
        bit3 m_base;
        bit3 m_index;
        bit2 m_scale;
    };

    class rex {
    public:
        rex() = default;
        rex(bit1 w, bit1 r, bit1 x, bit1 b) : m_w{w}, m_r{r}, m_x{x}, m_b{b}{}
        operator uint8_t() const {
            return (4 << 4) | (m_w << 3) | (m_r << 2) | (m_x << 1) | m_b;
        }

        auto w() const {
            return m_w;
        }
        auto r() const {
            return m_r;
        }
        auto x() const {
            return m_x;
        }
        auto b() const {
            return m_b;
        }

        auto set_w(bit1 v) &&{
            return rex{v, m_r, m_x, m_b};
        }
    private:
        bit1 m_b;
        bit1 m_x;
        bit1 m_r;
        bit1 m_w;
    };


    auto adc(register_type::al dst, uint8_t imm8) {
        return std::array<uint8_t, 2>{0x14, imm8};
    }
    auto adc(register_type::ax dst, uint16_t imm16) {
        return std::to_array({uint8_t{0x66}, uint8_t{0x15}, static_cast<uint8_t>(imm16), static_cast<uint8_t>(imm16>>8)});
    }
    auto adc(register_type::eax dst, uint32_t imm32) {
        return std::to_array({uint8_t{0x15}, static_cast<uint8_t>(imm32), static_cast<uint8_t>(imm32>>8), static_cast<uint8_t>(imm32>>16), static_cast<uint8_t>(imm32>>24)});
    }
    auto adc(register_type::rax dst, uint32_t imm32) {
        return std::to_array({uint8_t{rex{}.set_w(1)}, uint8_t{0x15}, static_cast<uint8_t>(imm32), static_cast<uint8_t>(imm32>>8), static_cast<uint8_t>(imm32>>16), static_cast<uint8_t>(imm32>>24)});
    }

    auto adc(register_type::reg8 dst, uint8_t imm8) {
        return std::to_array({uint8_t{0x80}, uint8_t{modrm{}.set_reg(2).set_rm(dst)}, imm8});
    }
    auto adc(register_type::mem8 dst, uint8_t imm8) {
        //return std::to_array({uint8_t{0x80}, uint8_t{modrm{}.set_reg(2)}, imm8});
    }
}
