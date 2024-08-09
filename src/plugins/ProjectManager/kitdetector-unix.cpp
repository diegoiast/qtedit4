#include "kitdetector.h"
#include <filesystem>
#include <unistd.h>

constexpr auto PLATFORM_PATH_DELIMITER = ':';

namespace KitDetector {

[[maybe_unused]] static auto
isCompilerAlreadyFound(const std::vector<KitDetector::ExtraPath> &detected, const std::string &cc)
    -> bool;

auto findCompilersUnixImpl(std::vector<KitDetector::ExtraPath> &detected,
                           const std::string path_env, std::string cc_name, std::string cxx_name)
    -> void {
    for (auto version = 4; version < 20; version++) {
        auto cc = std::string(cc_name) + "-" + std::to_string(version);
        auto ss = std::stringstream(path_env);
        auto dir = std::string();
        while (std::getline(ss, dir, ':')) {
            auto full_path = dir + std::filesystem::path::preferred_separator + cc;
            if (access(full_path.c_str(), X_OK) == 0) {
                if (isCompilerAlreadyFound(detected, full_path)) {
                    continue;
                }
                auto extraPath = KitDetector::ExtraPath();
                auto cxx = cxx_name + "-" + std::to_string(version);
                extraPath.name = cc;
                extraPath.compiler_path = full_path;
                extraPath.comment = "# detected " + cc;
                extraPath.command += "export CC=" + cc;
                extraPath.command += "\n";
                extraPath.command += "export CXX=" + cxx;
                detected.push_back(extraPath);
            }
        }
    }
}

auto findCompilersUnix(std::vector<KitDetector::ExtraPath> &detected) -> void {
    auto path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        return;
    }
    findCompilersUnixImpl(detected, path_env, "gcc", "g++");
    findCompilersUnixImpl(detected, path_env, "clang", "clang++");
}

auto findCompilerToolsUnix(std::vector<KitDetector::ExtraPath> &detected) -> void {
    // TODO - do we need to find special out of source tools in Unis?
}

} // namespace KitDetector
