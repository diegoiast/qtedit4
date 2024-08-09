#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace KitDetector {

struct ExtraPath {
    std::string name;
    std::string compiler_path;
    std::string comment;
    std::string command;
};

#ifdef _WIN32
constexpr auto platformUnix = !true;
constexpr auto platformWindows = true;
#else
constexpr auto platformUnix = true;
constexpr auto platformWindows = !true;
#endif

auto findCompilers() -> std::vector<ExtraPath>;
auto findQtVersions(bool unix_target) -> std::vector<ExtraPath>;
auto findCompilerTools() -> std::vector<ExtraPath>;

void generateKitFiles(const std::filesystem::path &path, const std::vector<ExtraPath> &tools,
                      const std::vector<ExtraPath> &compilers,
                      const std::vector<ExtraPath> &qtInstalls, bool unix_target);

} // namespace KitDetector