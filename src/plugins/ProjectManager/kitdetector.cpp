#include "kitdetector.h"
#include <filesystem>

#if defined(__unix__)
#include "kitdetector-unix.cpp"
constexpr auto HOME_DIR_ENV = "HOME";
constexpr auto BINARY_EXT = "";
#endif

#if defined(_WIN32)
#include "kitdetector-win32.cpp"
constexpr auto HOME_DIR_ENV = "USERPROFILE";
constexpr auto BINARY_EXT = ".exe";
#endif

namespace KitDetector {
/*
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
*/

auto isCompilerAlreadyFound(const std::vector<ExtraPath> &detected, const std::string &cc) -> bool {
    auto canonnical_cc = std::filesystem::canonical(cc);
    for (auto p : detected) {
        auto c = std::filesystem::canonical(p.compiler_path);
        if (c == canonnical_cc) {
            return true;
        }
    }
    return false;
}

auto replaceAll(std::string &str, const std::string &from, const std::string &to) -> void {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

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

auto isValidQtInstallation(const std::filesystem::path &path) -> bool {
    std::filesystem::path qmakePath = path / "bin" / (std::string("qmake") + BINARY_EXT);

    // check if this this is a false positite.
    auto ss = qmakePath.string();
    if (ss.find("qt6_design_studio_reduced_version") != std::string::npos) {
        return false;
    }
    return std::filesystem::exists(qmakePath);
}

auto findQtVersions() -> std::vector<ExtraPath> {
    auto knownLocations = std::vector<std::filesystem::path>{
        // clang-format off
        "/usr/local/Qt",
        "/opt/Qt",
        "c:\\qt\\"
        // clang-format on
    };

    auto homeDirEnv = getenv(HOME_DIR_ENV);
    if (homeDirEnv) {
        auto homedir = std::filesystem::path(homeDirEnv) / "qt";
        knownLocations.push_back(homedir);
    }

    auto detected = std::vector<ExtraPath>();
    for (const auto &root : knownLocations) {
        if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
            continue;
        }
        for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
            if (entry.is_directory()) {
                if (isValidQtInstallation(entry.path())) {
                    auto extraPath = ExtraPath();
                    extraPath.compiler_path = entry.path().string();
#if defined(__unix__)
                    extraPath.comment = "# qt installation";
                    extraPath.command = "export QTDIR=%1";
                    extraPath.command += "\n";
                    extraPath.command += "export QT_DIR=%1";
                    extraPath.command += "\n";
                    extraPath.command += "export PATH=$QTDIR/bin:$PATH";
#endif

#if defined(_WIN32)
                    extraPath.comment = "@rem qt installation";
                    extraPath.command = "SET QTDIR=%1";
                    extraPath.command += "\n";
                    extraPath.command += "SET QT_DIR=%1";
                    extraPath.command += "\n";
                    extraPath.command += "SET PATH=%QTDIR%\\bin:%PATH%";
#endif

                    replaceAll(extraPath.command, "%1", entry.path().string());
                    replaceAll(extraPath.comment, "%1", entry.path().string());
                    detected.push_back(extraPath);
                }
            }
        }
    }

    return detected;
}

} // namespace KitDetector
