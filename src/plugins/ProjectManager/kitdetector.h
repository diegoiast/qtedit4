#pragma once

#include <string>
#include <vector>

namespace KitDetector {

struct ExtraPath {
    std::string compiler_path;
    std::string comment;
    std::string command;
};

auto findCompilers() -> std::vector<ExtraPath>;
auto findQtVersions() -> std::vector<ExtraPath>;
auto findCompilerTools() -> std::vector<ExtraPath>;

} // namespace KitDetector