#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>

#include "general_purpose_register.hpp"

namespace amd64 {
    constexpr auto instruction_length_limit = 15;
    template<size_t N>
    struct bits {
        static_assert(N <= 8);
        constexpr bits() = default;
        constexpr bits(uint8_t v) : m_v{v}
        {
            assert(v < (1<<N));
        }
        constexpr operator uint8_t() const {
            return m_v;
        }
        uint8_t m_v : N;
    };

    using bit1 = bits<1>;
    using bit2 = bits<2>;
    using bit3 = bits<3>;

    template<typename Reg>
    struct base{
        Reg m_reg;
    };
    template<typename Imm>
    struct scale{
        Imm m_imm;
    };
    template<typename Reg>
    struct index{
        Reg m_reg;
    };
    template<typename Ref, typename... Refs>
    struct mem8 {
        Ref m_ref;
        mem8<Refs...> m_refs;
    };
    template<typename Ref>
    struct mem8<Ref> {
        Ref m_ref;
    };

    template<typename T>
    struct is_memory {
        constexpr static bool value = false;
    };
    template<typename... Refs>
    struct is_memory<mem8<Refs...>> {
        constexpr static bool value = true;
    };

    template<typename T>
    constexpr bool is_memory_v = is_memory<T>::value;

    template<typename T>
    concept memory = is_memory_v<T>;

    template<typename T>
    concept reg_or_mem = gpr<T> || memory<T>;

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
        auto set_reg(register_type::reg8 reg) && {
            return modrm{3, static_cast<uint8_t>(reg), m_rm};
        }
        auto set_reg(register_type::reg16 reg) && {
            return modrm{3, static_cast<uint8_t>(reg), m_rm};
        }
        auto set_reg(register_type::reg32 reg) && {
            return modrm{3, static_cast<uint8_t>(reg), m_rm};
        }
        auto set_reg(register_type::reg64 reg) && {
            return modrm{3, static_cast<uint8_t>(reg), m_rm};
        }

        template<size_t N>
        auto set_rm(register_type::ax_r<N> r) && {
            return modrm{3, m_reg, 0};
        }
        template<size_t N>
        auto set_rm(register_type::reg<N> r) && {
            return modrm{3, m_reg, static_cast<uint8_t>(r.r)};
        }

        auto set_rm(mem8<register_type::modrm_reg64_address> dst) &&{
            return modrm{0, m_reg, static_cast<uint8_t>(dst.m_ref)};
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

    constexpr auto sib_for(gpr auto reg) {
        return std::array<uint8_t, 0>{};
    }

    template<size_t N1, size_t N2>
    constexpr auto cat(std::array<uint8_t, N1> first, std::array<uint8_t, N2> second) {
        auto res = std::array<uint8_t, N1+N2>{};
        std::ranges::copy(first, res.begin());
        std::ranges::copy(second, res.begin()+N1);
        return res;
    }
    template<size_t N>
    constexpr auto cat(std::array<uint8_t, N> first, uint8_t second) {
        auto res = std::array<uint8_t, N+1>{};
        std::ranges::copy(first, res.begin());
        res[N] = second;
        return res;
    }
    constexpr auto cat(auto first, modrm second) {
        return cat(first, uint8_t{second});
    }
    constexpr auto cat(auto first, auto second, auto... others) {
        return cat(cat(first,second), others...);
    }
    template<size_t N>
    struct imm {
    };
    template<> struct imm<8> { using type = uint8_t; };
    template<> struct imm<16> { using type = uint16_t; };
    template<> struct imm<32> { using type = uint32_t; };
    template<> struct imm<64> { using type = uint64_t; };
    template<size_t N> using imm_t = imm<N>::type;

    auto to_codes(uint8_t imm8) { return std::array<uint8_t, 1>{imm8}; }
    auto to_codes(uint16_t imm16) {
        return std::array<uint8_t, 2>{
            static_cast<uint8_t>(imm16), static_cast<uint8_t>(imm16>>8) };
    }
    auto to_codes(uint32_t imm32) {
        return std::array<uint8_t, 4>{static_cast<uint8_t>(imm32), static_cast<uint8_t>(imm32>>8), static_cast<uint8_t>(imm32>>16), static_cast<uint8_t>(imm32>>24)};
    }

    auto prefix_for_16(auto imm) {
        return std::array<uint8_t, 0>{};
    }
    auto prefix_for_16(register_type::reg<16> reg) {
        return std::array<uint8_t, 1>{ 0x66 };
    }
    auto prefix_for_64(auto reg) {
        return std::array<uint8_t, 0>{};
    }
    auto prefix_for_64(register_type::reg<64> reg) {
        return std::array<uint8_t, 1>{ rex{}.set_w(1) };
    }
    template<size_t N>
    struct imm_for {
        using type = imm_t<N>;
    };
    template<size_t N>
    using imm_for_t = imm_for<N>::type;
    template<>
    struct imm_for<64> {
        using type = imm_t<32>;
    };

    template<typename T>
    struct imm_for_reg {
        using type = imm_t<T::size()>;
    };
    template<typename T>
    using imm_for_reg_t = imm_for_reg<T>::type;

    enum class operation : uint32_t {
        adc,
        add
    };

    constexpr auto ax_imm_opcode_map = std::to_array({
        [std::to_underlying(operation::adc)] = 0x15,
    });

    template<uint8_t Opcode, size_t N>
    auto ax_imm_instruction(register_type::ax_r<N> dst, imm_for_t<N> imm) {
        return cat(
                prefix_for_16(dst),
                prefix_for_64(dst),
                static_cast<uint8_t>(Opcode ^ (N==8)),
                to_codes(imm)
                );
    }

    struct regmem_imm_opcode {
        uint8_t opcode;
        bit3 modrm_reg;
    };

    template<uint8_t Opcode_ax_imm, regmem_imm_opcode Opcode_regmem_imm, uint8_t Opcode_regmem_reg>
    struct arithmetic_instruction {
        template<size_t N>
        constexpr static auto operator ()(register_type::ax_r<N> dst, std::integral auto imm) {
            imm_for_reg_t<register_type::ax_r<N>> i = imm;
            return ax_imm_instruction<Opcode_ax_imm, N>(dst, i);
        }
        constexpr static auto operator ()(reg_or_mem auto dst, std::integral auto imm) {
            imm_for_reg_t<std::remove_cvref_t<decltype(dst)>> i = imm;
            return cat(
                    prefix_for_16(dst),
                    prefix_for_64(dst),
                    static_cast<uint8_t>(Opcode_regmem_imm.opcode ^ (dst.size()==8)),
                    modrm{}.set_reg(Opcode_regmem_imm.modrm_reg).set_rm(dst),
                    sib_for(dst),
                    to_codes(i)
                    );
        }

        constexpr static auto operator ()(reg_or_mem auto dst, gpr auto src) {
            return cat(
                    prefix_for_16(dst),
                    prefix_for_64(dst),
                    static_cast<uint8_t>(Opcode_regmem_reg ^ (8 == src.size())),
                    modrm{}.set_reg(src.r).set_rm(dst),
                    sib_for(dst)
                    );
        }
    };
    constexpr auto adc      = arithmetic_instruction<0x15, {0x81,2}, 0x11>{};
    constexpr auto add      = arithmetic_instruction<0x05, {0x81,0}, 0x01>{};
    constexpr auto bit_and  = arithmetic_instruction<0x25, {0x81,4}, 0x21>{};
    constexpr auto cmp      = arithmetic_instruction<0x3d, {0x81,7}, 0x39>{};
    constexpr auto bit_or   = arithmetic_instruction<0x0d, {0x81,1}, 0x09>{};
    constexpr auto sbb      = arithmetic_instruction<0x1d, {0x81,3}, 0x19>{};
    constexpr auto sub      = arithmetic_instruction<0x2d, {0x81,5}, 0x29>{};
    constexpr auto test     = arithmetic_instruction<0xa9, {0xf7,0}, 0x85>{};
    constexpr auto bit_xor  = arithmetic_instruction<0x35, {0x81,6}, 0x31>{};

}
