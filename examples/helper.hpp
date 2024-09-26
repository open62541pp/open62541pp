#pragma once

#include <algorithm>  // find
#include <optional>
#include <string_view>
#include <vector>

class CliParser {
public:
    CliParser(int argc, char* argv[])
        : args_(argv, argv + argc) {}

    auto args() const noexcept {
        return args_;
    }

    auto nargs() const noexcept {
        return args_.size();
    }

    bool hasFlag(std::string_view flag) const {
        const auto it = std::find(args_.begin() + 1, args_.end(), flag);
        return it != args_.end();
    }

    std::optional<std::string_view> getValue(std::string_view option) const {
        auto it = std::find(args_.begin() + 1, args_.end(), option);
        if (it != args_.end()) {
            ++it;
        }
        if (it != args_.end()) {
            return *it;
        }
        return {};
    }

private:
    const std::vector<std::string_view> args_;
};
