#pragma once

#include <vector>
#include <unordered_map>
#include <string>

namespace symbol {

    template<typename Symbol>
    struct symbol_table {
        std::vector<Symbol> symbols;
        std::unordered_map<std::string, size_t> symbol_indices;

        void add_symbol(std::string name, Symbol symbol) {
            symbol_indices.emplace(name, symbols.size());
            symbols.emplace_back(symbol);
        }
        auto symbol_index(std::string name) {
            return symbol_indices[name];
        }
        auto operator [](size_t index) const{
            return symbols[index];
        }
    };
}
