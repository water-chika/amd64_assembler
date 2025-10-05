#pragma once

#include <iostream>
#include <optional>
#include <vector>
#include <cctype>

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
        std::istream& in;

        auto begin() {
        }
        auto end() {
        }

        auto next() {
        }
    };
}
