#include "kitdetector.h"
#include <filesystem>

namespace KitDetector {

#include <unistd.h>

#if 0
#if defined(__unix__)
constexpr auto PLATFORM_PATH_DELIMITER = ':';
#elif defined(_WIN32)
constexpr auto PLATFORM_PATH_DELIMITER = ';';
#endif

static auto is_command_in_path(const std::string &cmd,
                               const char delimiter = PLATFORM_PATH_DELIMITER) -> std::string {
    auto path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        return {};
    }

    auto ss = std::stringstream(path_env);
    auto dir = std::string();
    while (std::getline(ss, dir, delimiter)) {
        auto full_path = dir + std::filesystem::path::preferred_separator + cmd;
        if (access(full_path.c_str(), X_OK) == 0) {
            return full_path;
        }
    }

    return {};
}
#endif

auto isCompilerAlreadyFound(const std::vector<ExtraPath> &detected, const std::string &cc) {
    auto canonnical_cc = std::filesystem::canonical(cc);
    for (auto p : detected) {
        auto c = std::filesystem::canonical(p.compiler_path);
        if (c == canonnical_cc) {
            return true;
        }
    }
    return false;
}

#if defined(__unix__)
auto findCompilersLinuxImpl(std::vector<ExtraPath> &detected, const std::string path_env,
                            std::string cc_name, std::string cxx_name) -> void {
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
                auto extraPath = ExtraPath();
                auto cxx = cxx_name + "-" + std::to_string(version);
                extraPath.compiler_path = full_path;
                extraPath.comment = "# detected " + cc;
                extraPath.command = "export CC=" + cc;
                extraPath.command += "\n";
                extraPath.command = "export CXX=" + cxx;
                detected.push_back(extraPath);
            }
        }
    }
}

auto findCompilersLinux(std::vector<ExtraPath> &detected) -> void {
    auto path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        return;
    }
    findCompilersLinuxImpl(detected, path_env, "gcc", "g++");
    findCompilersLinuxImpl(detected, path_env, "clang", "clang++");
}
#endif

#if defined(_WIN32)

#include <shlobj.h>
#include <windows.h>

static auto checkVisualStudioVersion(const std::filesystem::path &basePath,
                                     const std::string &version, ExtraPath &extraPath) -> bool {
    std::filesystem::path versionPath = basePath / "Microsoft Visual Studio" / version;
    if (std::filesystem::exists(versionPath)) {
        extraPath.directory = versionPath;
        extraPath.name = "Visual Studio " + version;
        extraPath.command = "MSVC Tools";
        return true;
    }
    return false;
}

static auto findCompilersWindows(std::vector<ExtraPath> &detected) -> void {
    wchar_t programFiles[MAX_PATH];
    wchar_t programFiles86[MAX_PATH];
    HRESULT result;

    // clang-format off
    auto versions = std::vector<std::string>{
        "2022\\Community",
        "2022\\Professional",
        "2022\\Enterprise",
        "2019\\Community",
        "2019\\Professional",
        "2019\\Enterprise"
    };
    // clang-format on

    result = SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &path);
    if (!SUCCEEDED(result)) {
        programFiles[0] = 0;
    }

    result = SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &path);
    if (!SUCCEEDED(result)) {
        programFiles86[0] = 0;
    }

    // Check for each version in Program Files
    for (const auto &version : versions) {
        if (programFiles[0] && checkVisualStudioVersion(programFiles, version, extraPath)) {
            detected.push_back(extraPath);
        }
        if (programFiles86[0] && checkVisualStudioVersion(programFiles86, version, extraPath)) {
            detected.push_back(extraPath);
        }
    }
}
#endif

auto findCompilers() -> std::vector<ExtraPath> {
    auto detected = std::vector<ExtraPath>();

#if defined(__unix__)
    findCompilersLinux(detected);
#endif

#if defined(_WIN32)
    findCompilersWindows(detected);
#endif

    return detected;
}

} // namespace KitDetector
