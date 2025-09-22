#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "cpp_helper/cpp_helper.hpp"
#include "types.hpp"

#include "general_purpose_register.hpp"

namespace amd64 {
    constexpr auto instruction_length_limit = 15;

    using cpp_helper::bitset;

    using bit1 = bitset<1>;
    using bit2 = bitset<2>;
    using bit3 = bitset<3>;
    using bit4 = bitset<4>;
    using bit5 = bitset<5>;
    using bit6 = bitset<6>;
    using bit7 = bitset<7>;
    using bit8 = bitset<8>;

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
    template<size_t N, typename Ref = register_type::modrm_reg64_address>
    struct mem {
        Ref m_ref;

        constexpr static auto size() { return N; }
    };

    using mem8 = mem<8>;

    template<typename T>
    struct is_memory {
        constexpr static bool value = false;
    };
    template<size_t N, typename Ref>
    struct is_memory<mem<N, Ref>> {
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

        auto set_rm(mem8 dst) &&{
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
    constexpr auto sib_for(mem8) {
        return std::array<uint8_t, 0>{};
    }

    template<size_t N1, size_t N2>
    constexpr auto cat(std::array<uint8_t, N1> first, std::array<uint8_t, N2> second) {
        auto res = std::array<uint8_t, N1+N2>{};
        std::ranges::copy(first, res.begin());
        std::ranges::copy(second, res.begin()+N1);
        return res;
    }
    template<size_t N1, size_t N2>
    constexpr auto cat(std::array<uint8_t, N1> first, std::array<uint8_t, N2> second, auto... others) {
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
        add,
        sub,
        mul,
        div,
        mov,
        logic_and,
        logic_or,
        logic_xor,
        logic_not,
    };

    constexpr auto ax_imm_opcode_map = std::to_array({
        [std::to_underlying(operation::adc)] = 0x15,
    });

    template<uint8_t Opcode>
    constexpr auto gen_imm_instruction(std::integral auto imm) {
        return cat(
                prefix_for_16(imm),
                prefix_for_64(imm),
                std::array<uint8_t,1>{Opcode},
                to_codes(imm)
                );
    }

    template<uint8_t Opcode_for_8, auto Opcode>
    struct imm_instruction {
        template<typename Int>
            requires (std::integral<Int> && sizeof(Int) == 1)
        constexpr static auto operator ()(Int imm) {
            static_assert(sizeof(imm) == 1);
            return gen_imm_instruction<Opcode_for_8>(imm);
        }
        constexpr static auto operator ()(std::integral auto imm) {
            static_assert(sizeof(imm) == 2 || sizeof(imm) == 4 || sizeof(imm) == 8);
            return gen_imm_instruction<Opcode>(imm);
        }

        using support_argument_types = types::types<
            std::tuple<uint8_t>,
            std::tuple<uint16_t>,
            std::tuple<uint32_t>,
            std::tuple<uint64_t>
        >;
    };

    template<typename T1, typename... Ts>
    struct integrate_instructions : public cpp_helper::overloads<T1, Ts...>{
        constexpr integrate_instructions() = default;
        constexpr integrate_instructions(T1 t1, Ts... ts) : cpp_helper::overloads<T1, Ts...>(t1, ts...) {}
        using support_argument_types = types::add_types<
                                        typename T1::support_argument_types,
                                        typename Ts::support_argument_types...>;
        using cpp_helper::overloads<T1, Ts...>::operator();
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

        using support_argument_types = types::types<
                std::tuple<register_type::ax_r<8>, uint8_t>,
                std::tuple<register_type::ax_r<16>, uint16_t>,
                std::tuple<register_type::ax_r<32>, uint32_t>,
                std::tuple<register_type::ax_r<64>, uint64_t>
            >;
    };
    template<uint8_t Opcode>
    struct imm_ax_instruction {
        template<size_t N>
        constexpr static auto operator ()(std::integral auto imm, register_type::ax_r<N> ax) {
            imm_for_reg_t<register_type::ax_r<N>> i = imm;
            return imm_instruction<Opcode ^ (N==8), Opcode>{}(i);
        }

        using support_argument_types = types::types<
                std::tuple<uint8_t, register_type::ax_r<8>>,
                std::tuple<uint16_t, register_type::ax_r<16>>,
                std::tuple<uint32_t, register_type::ax_r<32>>,
                std::tuple<uint64_t, register_type::ax_r<64>>
            >;
    };

    template<uint8_t Opcode>
    constexpr auto gen_reg_imm_instruction(gpr auto reg, std::integral auto imm) {
        imm_for_reg_t<std::remove_cvref_t<decltype(reg)>> i = imm;
        auto reg_encode = static_cast<uint8_t>(reg);
        assert(reg_encode < 0x10);
        return cat(
                    prefix_for_16(reg),
                    prefix_for_64(reg),
                    std::array<uint8_t,1>{Opcode + (reg_encode & 0xf)},
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

        using support_argument_types = types::types<
                std::tuple<register_type::reg<8>, uint8_t>,
                std::tuple<register_type::reg<16>, uint16_t>,
                std::tuple<register_type::reg<32>, uint32_t>,
                std::tuple<register_type::reg<64>, uint64_t>
            >;
    };

    template<opcode_modrm_reg Opcode>
    constexpr auto gen_regmem_imm_instruction(reg_or_mem auto regmem, std::integral auto imm) {
            imm_for_reg_t<std::remove_cvref_t<decltype(regmem)>> i = imm;
            return cat(
                    prefix_for_16(regmem),
                    prefix_for_64(regmem),
                    std::array<uint8_t,1>{Opcode.opcode},
                    std::array<uint8_t,1>{modrm{}.set_reg(Opcode.modrm_reg).set_rm(regmem)},
                    sib_for(regmem),
                    to_codes(i)
                    );
    }

    template<opcode_modrm_reg Opcode, opcode_modrm_reg Opcode_for_8 = {Opcode.opcode^1, Opcode.modrm_reg}>
    struct regmem_imm_instruction {
        constexpr static auto operator ()(reg_or_mem auto regmem, uint8_t imm) {
            static_assert(regmem.size() == 8);
            return gen_regmem_imm_instruction<Opcode_for_8>(regmem, imm);
        }
        constexpr static auto operator ()(reg_or_mem auto regmem, std::integral auto imm) {
            static_assert(regmem.size() != 8);
            return gen_regmem_imm_instruction<Opcode>(regmem, imm);
        }
        using support_argument_types = types::types<
                std::tuple<register_type::reg<8>, uint8_t>,
                std::tuple<register_type::reg<16>, uint16_t>,
                std::tuple<register_type::reg<32>, uint32_t>,
                std::tuple<register_type::reg<64>, uint32_t>,
                std::tuple<mem<8>, uint8_t>,
                std::tuple<mem<16>, uint16_t>,
                std::tuple<mem<32>, uint32_t>,
                std::tuple<mem<64>, uint32_t>
            >;
    };

    template<uint8_t Opcode_ax_imm, opcode_modrm_reg Opcode_regmem_imm, uint8_t Opcode_regmem_reg>
    struct arithmetic_instruction
        :
            public ax_imm_instruction<Opcode_ax_imm>,
            public regmem_imm_instruction<
                    Opcode_regmem_imm>
    {
        using ax_imm_instruction<Opcode_ax_imm>::operator();
        using regmem_imm_instruction<
                    Opcode_regmem_imm>::operator();

        constexpr static auto operator ()(reg_or_mem auto dst, gpr auto src) {
            return cat(
                    prefix_for_16(dst),
                    prefix_for_64(dst),
                    std::array<uint8_t,1>{Opcode_regmem_reg ^ (8 == src.size())},
                    std::array<uint8_t,1>{modrm{}.set_reg(src.r).set_rm(dst)},
                    sib_for(dst)
                    );
        }
        using support_argument_types = types::types<>;
    };
    struct wrong_operands {
        constexpr static auto operator ()(auto... operands) {
            throw std::runtime_error{__FILE__ ":" "wrong operands"};
            return std::array<uint8_t, 0>{};
        }
        using support_argument_types = types::types<>;
    };
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
                    std::array<uint8_t,1>{Opcode}
                    );
        }
    };
    template<opcode_modrm_reg Opcode_regmem>
    constexpr auto gen_regmem_instruction(reg_or_mem auto regmem) {
            return cat(
                    prefix_for_16(regmem),
                    prefix_for_64(regmem),
                    std::array<uint8_t, 1>{Opcode_regmem.opcode},
                    std::array<uint8_t, 1>{modrm{}.set_reg(Opcode_regmem.modrm_reg).set_rm(regmem)},
                    sib_for(regmem)
                    );
    }

    template<opcode_modrm_reg Opcode_regmem,
        bool support_8 = true,
        opcode_modrm_reg Opcode_regmem_for_8 = {Opcode_regmem.opcode^1, Opcode_regmem.modrm_reg}>
    struct regmem_instruction {
        template<typename T>
            requires (reg_or_mem<T> && T::size() == 8 && support_8)
        constexpr static auto operator ()(T regmem) {
            static_assert(regmem.size() == 8);
            return gen_regmem_instruction<Opcode_regmem>(regmem);
        }
        template<typename T>
            requires (reg_or_mem<T> && T::size() != 8)
        constexpr static auto operator ()(T regmem) {
            static_assert(regmem.size() == 16 || regmem.size() == 32 || regmem.size() == 64);
            return gen_regmem_instruction<Opcode_regmem>(regmem);
        }
    };

    template<uint8_t Opcode>
    struct reg_mem_instruction {
        constexpr static auto operator ()(gpr auto target, memory auto src) {
            static_assert(target.size() == 16 || target.size() == 32 || target.size() == 64);
            return cat(
                    prefix_for_16(target),
                    prefix_for_64(target),
                    std::array<uint8_t, 1>{Opcode},
                    std::array<uint8_t, 1>{modrm{}.set_reg(target).set_rm(src)},
                    sib_for(src)
                    );
        }
    };

    template<uint8_t Opcode>
    constexpr auto gen_regmem_reg_instruction(reg_or_mem auto regmem, gpr auto reg) {
            static_assert(regmem.size() == reg.size());
            return cat(
                    prefix_for_16(reg),
                    prefix_for_64(reg),
                    std::array<uint8_t, 1>{(Opcode ^ (reg.size() == 8))},
                    std::array<uint8_t, 1>{modrm{}.set_reg(reg).set_rm(regmem)},
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
    template<uint8_t Opcode>
    struct reg_reg_instruction {
        constexpr static auto operator ()(gpr auto regmem, gpr auto reg) {
            return gen_regmem_reg_instruction<Opcode>(regmem, reg);
        }
    };
}
