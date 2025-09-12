#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cassert>
#include <algorithm>

namespace amd64 {
    constexpr auto instruction_length_limit = 15;

    namespace register_type {

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
        enum class extended_reg8 : uint8_t {
        };
        enum class reg16 : uint8_t {
            ax = 0,
            cx = 1,
            dx = 2,
            bx = 3,
            sp = 4,
            bp = 5,
            si = 6,
            di = 7,
        };
        enum class extended_reg16 : uint8_t {};
        enum class reg32 : uint8_t {
            eax = 0,
            ecx = 1,
            edx = 2,
            ebx = 3,
            esp = 4,
            ebp = 5,
            esi = 6,
            edi = 7,
        };
        enum class extended_reg32 : uint8_t {};
        enum class reg64 : uint8_t {
            rax = 0,
            rcx = 1,
            rdx = 2,
            rbx = 3,
            rsp = 4,
            rbp = 5,
            rsi = 6,
            rdi = 7,
        };

        template<size_t N>
        struct ax_r {
            constexpr auto size() { return N; }
        };

        enum class extended_reg64 : uint8_t {};
        enum class extention : uint8_t {
            legacy,
            extended
        };
        template<size_t N, extention Ext = extention::legacy>
        struct reg {
        };
        template<> struct reg<8> {
            reg8 r;
            constexpr auto size() { return 8; }
        };
        template<> struct reg<16> {
            reg16 r;
            constexpr auto size() { return 16; }
        };
        template<> struct reg<32> {
            constexpr reg() = default;
            constexpr reg(reg32 r) : r{r}{}
            constexpr reg(ax_r<32> t) : r{ reg32::eax } {}
            
            constexpr auto size() { return 32; }
            reg32 r;
        };
        template<> struct reg<64> {
            constexpr auto size() { return 64; }
            reg64 r; };
        
        template<> struct reg<8, extention::extended> {
            constexpr auto size() { return 8; }
            extended_reg8 r; };
        template<> struct reg<16, extention::extended> {
            constexpr auto size() { return 16; }
            extended_reg16 r; };
        template<> struct reg<32, extention::extended> {
            constexpr auto size() { return 32; }
            extended_reg32 r; };
        template<> struct reg<64, extention::extended> {
            constexpr auto size() { return 64; }
            extended_reg64 r; };



        enum class modrm_reg64_address : uint8_t {
            rax = 0,
            rcx = 1,
            rdx = 2,
            rbx = 3,
            rbp = 5,
            rsi = 6,
            rdi = 7,
        };
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
    }

    constexpr auto al = register_type::ax_r<8>{};
    constexpr auto ax = register_type::ax_r<16>{};
    constexpr auto eax = register_type::ax_r<32>{};
    constexpr auto rax = register_type::ax_r<64>{};

    constexpr auto ebx = register_type::reg<32>{ register_type::reg32::ebx };

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

        auto set_rm(register_type::mem8<register_type::modrm_reg64_address> dst) &&{
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

    template<size_t N>
    auto adc(register_type::ax_r<N> dst, imm_for_t<N> imm) {
        return cat(
                prefix_for_16(dst),
                prefix_for_64(dst),
                static_cast<uint8_t>(0x15 ^ (N==8)),
                to_codes(imm)
                );
    }

    template<size_t N>
    auto adc(register_type::reg<N> dst, imm_for_t<N> imm) {
        return cat(
                prefix_for_16(dst),
                prefix_for_64(dst),
                static_cast<uint8_t>(0x81 ^ (N==8)),
                modrm{}.set_reg(2).set_rm(dst),
                to_codes(imm)
                );
    }
    auto adc(register_type::mem8<register_type::modrm_reg64_address> dst, uint8_t imm8) {
        return std::to_array({uint8_t{0x80}, uint8_t{modrm{}.set_reg(2).set_rm(dst)}, imm8});
    }

    template<typename T>
    concept reg_c = (std::same_as<T, register_type::ax_r<32>> || std::same_as<T, register_type::reg<32>>);

    auto adc(reg_c auto dst, reg_c auto src) {
        return cat(
                prefix_for_16(dst),
                prefix_for_64(dst),
                static_cast<uint8_t>(0x11 ^ (8 == src.size())),
                modrm{}.set_reg(src.r).set_rm(dst)
                );
    }
}
