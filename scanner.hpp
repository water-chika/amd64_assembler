#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <cctype>
#include <unordered_map>

template<>
struct std::hash<std::vector<char>> {

    size_t operator()(const std::vector<char>& v) const {
        size_t s = v.size() << 2;
        for (auto c : v) {
            s ^= c;
            s += c;
        }
        return s;
    }
};

namespace scanner {
    struct word_splitter {
        std::istream& in;

        std::optional<std::vector<char>> next() {
            std::vector<char> str{};

            auto ch = in.peek();
            while (in) {
                ch = in.get();
                if (!isspace(ch)) {
                    break;
                }
            }
            while (in) {
                str.emplace_back(ch);
                ch = in.get();
                if (isspace(ch)) {
                    break;
                }
            }
            if (str.size() == 0) {
                return std::nullopt;
            }
            return str;
        }
    };
    struct scanner {
        word_splitter splitter;
        std::vector<std::vector<char>> strings;
        std::unordered_map<std::vector<char>, size_t> string_indices;

        auto get_string(size_t i) {
            return strings[i];
        }
        auto add_string(std::vector<char> str) {
            auto i = strings.size();
            strings.emplace_back(str);
            string_indices.emplace(str, i);
            return i;
        }
        auto contains(const std::vector<char>& str) {
            return string_indices.contains(str);
        }
        auto string_index(const std::vector<char>& str) {
            return string_indices[str];
        }

        std::optional<size_t> next() {
            auto word_opt = splitter.next();
            if (!word_opt) {
                return std::nullopt;
            }
            else {
                auto word = word_opt.value();
                if (!contains(word)) {
                    add_string(word);
                }
                auto i = string_index(word);
                return i;
            }
        }
    };
}
