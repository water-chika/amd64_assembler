#pragma once

namespace amd64{
    template<typename T>
    struct is_gpr {
        constexpr static bool value = false;
    };
    template<typename T>
    constexpr bool is_gpr_v = is_gpr<T>::value;
    template<typename T>
    concept general_purpose_register = is_gpr_v<T>;
    template<typename T>
    concept gpr = general_purpose_register<T>;

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

        enum class extended_reg64 : uint8_t {};

        enum class extention : uint8_t {
            legacy,
            extended
        };
        template<size_t N, extention Ext = extention::legacy>
        struct reg {
        };

        template<size_t N>
        struct ax_r {
            static constexpr auto size() { return N; }
            operator reg<N>() {
                return {0};
            }
        };



        template<> struct reg<8> {
            reg8 r;
            static constexpr auto size() { return 8; }
            operator reg8() {
                return r;
            }
        };
        template<> struct reg<16> {
            reg16 r;
            static constexpr auto size() { return 16; }
            operator reg16() {
                return r;
            }
        };
        template<> struct reg<32> {
            constexpr reg() = default;
            constexpr reg(reg32 r) : r{r}{}
            constexpr reg(ax_r<32> t) : r{ reg32::eax } {}
            
            constexpr static auto size() { return 32; }
            reg32 r;

            operator reg32() {
                return r;
            }
        };
        template<> struct reg<64> {
            static constexpr auto size() { return 64; }
            reg64 r;

            operator reg64() {
                return r;
            }
        };
        
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
    }

    template<size_t N, register_type::extention Ext>
    struct is_gpr<register_type::reg<N, Ext>> {
        constexpr static bool value = true;
    };
    template<size_t N>
    struct is_gpr<register_type::ax_r<N>> {
        constexpr static bool value = true;
    };

    constexpr auto al = register_type::ax_r<8>{};
    constexpr auto ax = register_type::ax_r<16>{};
    constexpr auto eax = register_type::ax_r<32>{};
    constexpr auto rax = register_type::ax_r<64>{};

    constexpr auto ebx = register_type::reg<32>{ register_type::reg32::ebx };
}
