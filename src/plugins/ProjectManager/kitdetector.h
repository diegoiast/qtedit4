/**
 * \file kitdefinitions.cpp
 * \brief Definition of kit detector
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License MIT
 */

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
[[maybe_unused]] constexpr auto platformUnix = !true;
[[maybe_unused]] constexpr auto platformWindows = true;
#else
[[maybe_unused]] constexpr auto platformUnix = true;
[[maybe_unused]] constexpr auto platformWindows = !true;
#endif

auto findCompilers(bool unix_target) -> std::vector<ExtraPath>;
auto findQtVersions(bool unix_target) -> std::vector<ExtraPath>;
auto findCompilerTools(bool unix_target) -> std::vector<ExtraPath>;

void generateKitFiles(const std::filesystem::path &path, const std::vector<ExtraPath> &tools,
                      const std::vector<ExtraPath> &compilers,
                      const std::vector<ExtraPath> &qtInstalls, bool unix_target);

} // namespace KitDetector
