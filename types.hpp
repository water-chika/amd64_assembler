#pragma once

namespace types {
    template<typename... Ts>
    struct types {
        template<typename... Rhs>
        auto operator+(types<Rhs...> rhs) {
            return types<Ts..., Rhs...>{};
        }
    };

    template<typename Lhs, typename... Rhs>
    struct add_types {
        using type = decltype(Lhs{} + add_types<Rhs...>{});
    };
    template<typename Lhs, typename... Rhs>
    using add_types_t = add_types<Lhs, Rhs...>::type;
}

#include <variant>

namespace types {
    template<typename... Ts>
    auto to_variant(types<Ts...>) {
        return std::variant<Ts...>{};
    }

    template<typename Types>
    using to_variant_t = decltype(to_variant(Types{}));
}
