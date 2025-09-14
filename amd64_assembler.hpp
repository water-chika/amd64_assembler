#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>

#include "cpp_helper/cpp_helper.hpp"

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
    auto to_codes(int8_t imm8) { return to_codes(static_cast<uint8_t>(imm8));}

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

    template<uint8_t Opcode_for_8, auto Opcode_for_16_32>
    struct imm_instruction {
        constexpr static auto operator ()(int8_t imm) {
            return cat(
                    prefix_for_16(imm),
                    prefix_for_64(imm),
                    Opcode_for_8,
                    to_codes(imm)
                    );
        }
        template<typename T>
        constexpr static auto operator ()(T imm) {
            return cat(
                    prefix_for_16(imm),
                    prefix_for_64(imm),
                    Opcode_for_16_32,
                    to_codes(imm)
                    );
        }
    };

    struct opcode_modrm_reg {
        uint8_t opcode;
        bit3 modrm_reg;
    };

    template<uint8_t Opcode>
    struct ax_imm_instruction {
        template<size_t N>
        constexpr static auto operator ()(register_type::ax_r<N> dst, std::integral auto imm) {
            imm_for_reg_t<register_type::ax_r<N>> i = imm;
            return imm_instruction<Opcode ^ (N==8), Opcode>{}(i);
        }
    };
    template<uint8_t Opcode>
    struct imm_ax_instruction {
        template<size_t N>
        constexpr static auto operator ()(std::integral auto imm, register_type::ax_r<N> ax) {
            imm_for_reg_t<register_type::ax_r<N>> i = imm;
            return imm_instruction<Opcode ^ (N==8), Opcode>{}(i);
        }
    };

    template<uint8_t Opcode>
    constexpr auto gen_reg_imm_instruction(gpr auto reg, std::integral auto imm) {
        imm_for_reg_t<std::remove_cvref_t<decltype(reg)>> i = imm;
        auto reg_encode = static_cast<uint8_t>(reg);
        assert(reg_encode < 0x10);
        return cat(
                    prefix_for_16(reg),
                    prefix_for_64(reg),
                    Opcode + (reg_encode & 0xf),
                    to_codes(i)
                );
    }

    template<uint8_t Opcode_for_8, uint8_t Opcode>
    struct reg_imm_instruction {
        constexpr static auto operator ()(gpr auto reg, uint8_t imm) {
            static_assert(reg.size() == 8);
            return gen_reg_imm_instruction<Opcode_for_8>(reg, imm);
        }
        constexpr static auto operator ()(gpr auto reg, std::integral auto imm) {
            static_assert(reg.size() != 8);
            return gen_reg_imm_instruction<Opcode>(reg, imm);
        }
    };

    template<opcode_modrm_reg Opcode>
    constexpr auto gen_regmem_imm_instruction(reg_or_mem auto regmem, std::integral auto imm) {
            imm_for_reg_t<std::remove_cvref_t<decltype(regmem)>> i = imm;
            return cat(
                    prefix_for_16(regmem),
                    prefix_for_64(regmem),
                    Opcode.opcode,
                    modrm{}.set_reg(Opcode.modrm_reg).set_rm(regmem),
                    sib_for(regmem),
                    to_codes(i)
                    );
    }

    template<opcode_modrm_reg Opcode_for_8, opcode_modrm_reg Opcode>
    struct regmem_imm_instruction {
        constexpr static auto operator ()(reg_or_mem auto regmem, uint8_t imm) {
            static_assert(regmem.size() == 8);
            return gen_regmem_imm_instruction<Opcode_for_8>(regmem, imm);
        }
        constexpr static auto operator ()(reg_or_mem auto regmem, std::integral auto imm) {
            static_assert(regmem.size() != 8);
            return gen_regmem_imm_instruction<Opcode>(regmem, imm);
        }
    };

    template<uint8_t Opcode_ax_imm, opcode_modrm_reg Opcode_regmem_imm, uint8_t Opcode_regmem_reg>
    struct arithmetic_instruction
        :
            public ax_imm_instruction<Opcode_ax_imm>,
            public regmem_imm_instruction<
                    { Opcode_regmem_imm.opcode^1, Opcode_regmem_imm.modrm_reg },
                    Opcode_regmem_imm>
    {
        using ax_imm_instruction<Opcode_ax_imm>::operator();
        using regmem_imm_instruction<
                    { Opcode_regmem_imm.opcode^1, Opcode_regmem_imm.modrm_reg },
                    Opcode_regmem_imm>::operator();

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

    template<uint8_t Opcode, typename T>
        requires std::same_as<T,int16_t> || std::same_as<T, int32_t>
    auto imm_16_32_instruction(T imm) {
        return cat(
                prefix_for_16(imm),
                prefix_for_64(imm),
                Opcode,
                to_codes(imm)
                );
    }

    template<uint8_t Opcode_imm, opcode_modrm_reg Opcode_regmem>
    struct call_instruction {
        constexpr static auto operator ()(std::integral auto imm) {
            return imm_16_32_instruction<Opcode_imm, std::remove_cvref_t<decltype(imm)>>(imm);
        }
        constexpr static auto operator ()(reg_or_mem auto target) {
            static_assert(target.size() == 16 || target.size() == 32 || target.size() == 64);
            return cat(
                    prefix_for_16(target),
                    prefix_for_64(target),
                    Opcode_regmem.opcode,
                    modrm{}.set_reg(Opcode_regmem.modrm_reg).set_rm(target),
                    sib_for(target)
                    );
        }
    };
    constexpr auto call = call_instruction<0xe8, {0xff,2}>{};

    template<auto Opcode, size_t N=32>
    struct opcode_instruction {
        constexpr static auto operator ()() {
            return cat(
                    prefix_for_16(imm_t<N>{}),
                    prefix_for_64(imm_t<N>{}),
                    Opcode
                    );
        }
    };
    constexpr auto cwb  = opcode_instruction<0x98, 16>{};
    constexpr auto cwde = opcode_instruction<0x98, 32>{};
    constexpr auto cdqe = opcode_instruction<0x98, 64>{};

    constexpr auto cwd = opcode_instruction<0x99, 16>{};
    constexpr auto cdq = opcode_instruction<0x99, 32>{};
    constexpr auto cqo = opcode_instruction<0x99, 64>{};

    constexpr auto clc = opcode_instruction<0xf8>{};
    constexpr auto cld = opcode_instruction<0xfc>{};

    constexpr auto clzero = opcode_instruction<std::to_array({0x0f, 0x01, 0xfc})>{};

    constexpr auto cmc = opcode_instruction<0xf5>{};

    constexpr auto cpuid = opcode_instruction<std::to_array({0x0f, 0xa2})>{};

    template<opcode_modrm_reg Opcode_regmem>
    struct regmem_instruction {
        constexpr static auto operator ()(reg_or_mem auto target) {
            static_assert(target.size() == 16 || target.size() == 32 || target.size() == 64);
            return cat(
                    prefix_for_16(target),
                    prefix_for_64(target),
                    Opcode_regmem.opcode,
                    modrm{}.set_reg(Opcode_regmem.modrm_reg).set_rm(target),
                    sib_for(target)
                    );
        }
    };

    constexpr auto div  = regmem_instruction<{0xf7, 6}>{};
    constexpr auto idiv = regmem_instruction<{0xf7, 7}>{};
    constexpr auto imul = regmem_instruction<{0xf7, 5}>{};

    template<bits<4> Condition_code>
    using jcc_instruction = imm_instruction<0x70 | Condition_code, std::to_array({0x0f, 0x80 | Condition_code})>;

    template<bits<4> Condition_code>
    constexpr auto jcc = jcc_instruction<Condition_code>{};

    constexpr auto jo  = jcc_instruction<0x0>{};
    constexpr auto jno = jcc_instruction<0x1>{};
    constexpr auto jb  = jcc_instruction<0x2>{};
    constexpr auto jnb = jcc_instruction<0x3>{};
    constexpr auto jz  = jcc_instruction<0x4>{};
    constexpr auto jnz = jcc_instruction<0x5>{};
    constexpr auto jbe = jcc_instruction<0x6>{};
    constexpr auto jnbe= jcc_instruction<0x7>{};
    constexpr auto js  = jcc_instruction<0x8>{};
    constexpr auto jns = jcc_instruction<0x9>{};
    constexpr auto jp  = jcc_instruction<0xa>{};
    constexpr auto jnp = jcc_instruction<0xb>{};
    constexpr auto jl  = jcc_instruction<0xc>{};
    constexpr auto jnl = jcc_instruction<0xd>{};
    constexpr auto jle = jcc_instruction<0xe>{};
    constexpr auto jnle= jcc_instruction<0xf>{};

    constexpr auto jmp = cpp_helper::overloads<imm_instruction<0xeb, 0xe9>,regmem_instruction<{0xff, 4}>>{};

    template<uint8_t Opcode>
    struct reg_mem_instruction {
        constexpr static auto operator ()(gpr auto target, memory auto src) {
            static_assert(target.size() == 16 || target.size() == 32 || target.size() == 64);
            return cat(
                    prefix_for_16(target),
                    prefix_for_64(target),
                    Opcode,
                    modrm{}.set_reg(target).set_rm(src),
                    sib_for(src)
                    );
        }
    };
    constexpr auto lea = reg_mem_instruction<0x8d>{};

    template<uint8_t Opcode>
    constexpr auto gen_regmem_reg_instruction(reg_or_mem auto regmem, gpr auto reg) {
            static_assert(regmem.size() == reg.size());
            return cat(
                    prefix_for_16(reg),
                    prefix_for_64(reg),
                    static_cast<uint8_t>(Opcode ^ (reg.size() == 8)),
                    modrm{}.set_reg(reg).set_rm(regmem),
                    sib_for(regmem)
                    );
    }

    template<uint8_t Opcode>
    struct regmem_reg_instruction {
        constexpr static auto operator ()(reg_or_mem auto regmem, gpr auto reg) {
            return gen_regmem_reg_instruction<Opcode>(regmem, reg);
        }
    };
    template<uint8_t Opcode>
    struct reg_regmem_instruction {
        constexpr static auto operator ()(gpr auto reg, reg_or_mem auto regmem) {
            return gen_regmem_reg_instruction<Opcode>(regmem, reg);
        }
    };

    constexpr
    auto mov = cpp_helper::overloads<
        regmem_reg_instruction<0x89>,
        reg_regmem_instruction<0x8b>,
        ax_imm_instruction<0xa1>,
        imm_ax_instruction<0xa3>,
        reg_imm_instruction<0xb0, 0xb8>
    >{};
}
