/**
 * \file kitdefinitions.cpp
 * \brief Implementation of kit detector
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * SPDX-License-Identifier: MIT
 */

#include "kitdetector.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>

#if defined(__unix__)
#include <sys/stat.h>
#include <unistd.h>

constexpr auto HOME_DIR_ENV = "HOME";
constexpr auto BINARY_EXT = "";
constexpr auto ENV_SEPARATOR = ':';
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <functional>
#include <qDebug>
#include <shlobj.h>
#include <windows.h>

constexpr auto HOME_DIR_ENV = "USERPROFILE";
constexpr auto BINARY_EXT = ".exe";
constexpr auto ENV_SEPARATOR = ';';
#endif

constexpr auto SCRIPT_EXTENSION_UNIX = ".sh";
constexpr auto SCRIPT_HEADER_UNIX = R"(#! /bin/sh

# This is a kit definition for qtedit4. All your tasks
# will run through this file.

# available enritonment variables:
# ${source_directory} - where the source being executed is saved
# ${build_directory}  - where code should be compile into
# ${run_directory}    - where this task should be run, as defined in
#                       the task's definition in the JSON file
# ${task}             - the actual code to be run

# The following meta variables are for qtedit4. Note the prefix:
# @@ name = @@NAME@@
# @@ author = auto generated - by qtedit4

# from this point on - echo is on. Every command will be displayed
# in the build output.
set -x
set -e

echo "Running from kit ${0}"
echo "Source is in        : ${source_directory}"
echo "Binaries will be    : ${build_directory}"
echo "Working directory is: ${run_directory}"
)";
constexpr auto SCRIPT_SUFFIX_UNIX = R"(# execute task
cd ${run_directory}
${task}
)";

constexpr auto SCRIPT_EXTENSION_WIN32 = ".bat";
constexpr auto SCRIPT_HEADER_WIN32 = R"(@echo off

rem This is a kit definition for qtedit4. All your tasks
rem will run through this file.

rem available enritonment variables:
rem %source_directory% - where the source being executed is saved
rem %build_directory%  - where code should be compile into
rem %run_directory%    - where this task should be run, as defined in
rem                      the task's definition in the JSON file
rem %task%             - the actual code to be run

rem The following meta variables are for qtedit4. Note the prefix:
rem @@ name = @@NAME@@
rem @@ author = auto generated - by qtedit4

rem from this point on - echo is on. Every command will be displayed
rem in the build output.
@echo on

@echo "Running from kit %0%"
@echo "Source is in        : %source_directory%"
@echo "Binaries will be in : %build_directory%"
@echo "Working directory is: %run_directory%"

)";
constexpr auto SCRIPT_SUFFIX_WIN32 = R"(
@rem execute task
cd %run_directory%
%task%
)";

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

static auto replaceAll(std::string &str, const std::string &from, const std::string &to)
    -> std::string & {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

#if defined(_WIN32)
static auto wstringToString(const std::wstring &wstr) -> std::string {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0,
                                          nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &str[0], size_needed, nullptr,
                        nullptr);
    return str;
}

static auto checkVisualStudioVersion(PWSTR basePath, const std::wstring &version,
                                     KitDetector::ExtraPath &extraPath) -> bool {
    std::wstring basePathW(basePath);
    std::filesystem::path versionPath =
        std::filesystem::path(basePathW) / "Microsoft Visual Studio" / version;
    if (std::filesystem::exists(versionPath)) {
        extraPath.name = wstringToString(version);
        extraPath.compiler_path = wstringToString(versionPath);
        extraPath.comment = "@rem VS " + wstringToString(version);
        extraPath.command = "call %1\\VC\\Auxiliary\\Build\\vcvarsall.bat";
        KitDetector::replaceAll(extraPath.command, "%1", versionPath.string());
        return true;
    }
    return false;
}

static auto findCompilersWindows(std::vector<KitDetector::ExtraPath> &detected) -> void {
    PWSTR programFiles = nullptr;
    PWSTR programFiles86 = nullptr;

    SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &programFiles);
    SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &programFiles86);

    // clang-format off
    auto versions = std::vector<std::wstring>{
        L"2022\\Community",
        L"2022\\Professional",
        L"2022\\Enterprise",
        L"2019\\Community",
        L"2019\\Professional",
        L"2019\\Enterprise"
    };
    // clang-format on

    for (const auto &version : versions) {
        auto extraPath = KitDetector::ExtraPath();
        if (programFiles[0] && checkVisualStudioVersion(programFiles, version, extraPath)) {
            detected.push_back(extraPath);
        }
        if (programFiles86[0] && checkVisualStudioVersion(programFiles86, version, extraPath)) {
            detected.push_back(extraPath);
        }
    }
    CoTaskMemFree(programFiles86);
    CoTaskMemFree(programFiles);
}

static auto findCompilerToolsWindows(std::vector<KitDetector::ExtraPath> &detected) -> void {
    auto cmakePath = std::filesystem::path();
    PWSTR programFiles = nullptr;
    PWSTR programFiles86 = nullptr;

    SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &programFiles);
    cmakePath = std::filesystem::path(programFiles) / "CMake" / "bin" / "cmake.exe";
    if (std::filesystem::exists(cmakePath)) {
        auto extraPath = KitDetector::ExtraPath();
        extraPath.name = "CMake";
        extraPath.compiler_path = (std::filesystem::path(programFiles) / "CMake" / "bin").string();
        extraPath.comment = "@rem Found CMake";
        extraPath.command = "set PATH=" + extraPath.compiler_path + ";%PATH%";
        detected.push_back(extraPath);
    }

    SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &programFiles86);
    cmakePath = std::filesystem::path(programFiles86) / "CMake" / "bin" / "cmake.exe";
    if (std::filesystem::exists(cmakePath)) {
        auto extraPath = KitDetector::ExtraPath();
        extraPath.name = "CMake (x86)";
        extraPath.compiler_path =
            (std::filesystem::path(programFiles86) / "CMake" / "bin").string();
        extraPath.comment = "@rem Found CMake (x86)";
        extraPath.command = "set PATH=" + extraPath.compiler_path + ";%PATH%";
        detected.push_back(extraPath);
    }
}
#endif

[[maybe_unused]] static auto isCompilerAlreadyFound(const std::vector<ExtraPath> &detected,
                                                    const std::string &cc) -> bool {
    auto canonnical_cc = std::filesystem::canonical(cc);
    for (const auto &p : detected) {
        auto c = std::filesystem::canonical(p.compiler_path);
        if (c == canonnical_cc) {
            return true;
        }
    }
    return false;
}

static auto safeGetEnv(const char *name) -> std::string {
#if defined(_WIN32)
    // TODO: we deal with ansi only - is this OK?
    // Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx
    DWORD bufferSize = 65535;
    std::string buff;
    buff.resize(bufferSize);
    bufferSize = GetEnvironmentVariableA(name, &buff[0], bufferSize);
    if (bufferSize > 0) {
        buff.resize(bufferSize);
    }
    return buff;
#else
    auto v = std::getenv(name);
    return v ? std::string(v) : std::string();
#endif
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

using PathCallback = std::function<void(const std::filesystem::path &path,
                                        const std::filesystem::path &full_filename)>;

static auto findCommandInPath(const std::string &cmd, PathCallback callback) -> void {
    auto path_env = safeGetEnv("PATH");
    if (path_env.empty()) {
        return;
    }
    auto ss = std::stringstream(path_env);
    auto dir = std::string();
    while (std::getline(ss, dir, ENV_SEPARATOR)) {
        auto full_path = std::filesystem::path(dir) / std::filesystem::path(cmd);
#if defined(__unix__)
        if (access(full_path.c_str(), X_OK) != 0) {
            continue;
        }
#else
        if (_waccess(full_path.c_str(), 04) != 0) {
            continue;
        }
#endif
        callback(std::filesystem::path(dir), full_path);
    }
    return;
}

auto static findCompilersImpl(std::vector<KitDetector::ExtraPath> &detected,
                              const std::string path_env, std::string cc_name, std::string cxx_name,
                              bool unix_target) -> void {
    for (auto version = 4; version < 20; version++) {
        auto cc = std::string(cc_name) + "-" + std::to_string(version) + BINARY_EXT;
        auto ss = std::stringstream(path_env);
        auto dir = std::string();

        findCommandInPath(
            cc, [version, &cc, &detected, &cxx_name, unix_target](auto path, auto full_path) {
                if (isCompilerAlreadyFound(detected, full_path.string())) {
                    return;
                }

                auto extraPath = KitDetector::ExtraPath();
                auto cxx = cxx_name + "-" + std::to_string(version);
                if (unix_target) {
                    extraPath.name = cc;
                    extraPath.compiler_path = full_path.string();
                    extraPath.comment = "# detected " + full_path.string();
                    extraPath.command += "export CC=" + cc;
                    extraPath.command += "\n";
                    extraPath.command += "export CXX=" + cxx;
                } else {
                    extraPath.name = cc;
                    extraPath.compiler_path = full_path.string();
                    extraPath.comment = "@rem detected " + full_path.string();
                    extraPath.command += "SET CC=" + cc;
                    extraPath.command += "\n";
                    extraPath.command += "SET CXX=" + cxx;
                }
                detected.push_back(extraPath);
            });
    }
}

auto static findCompilersInPath(std::vector<KitDetector::ExtraPath> &detected, bool unix_target)
    -> void {
    auto path_env = safeGetEnv("PATH");
    if (path_env.empty()) {
        return;
    }
    findCompilersImpl(detected, path_env, "gcc", "g++", unix_target);
    findCompilersImpl(detected, path_env, "clang", "clang++", unix_target);
}

////////////////////////////////////////////
// public API

auto findCompilers(bool unix_target) -> std::vector<ExtraPath> {
    auto detected = std::vector<ExtraPath>();
    findCompilersInPath(detected, unix_target);

#if defined(_WIN32)
    findCompilersWindows(detected);
#endif

    return detected;
}

auto findQtVersions(bool unix_target) -> std::vector<ExtraPath> {
    auto knownLocations = std::vector<std::filesystem::path>{
        // clang-format off
        "/usr/local/Qt",
        "/opt/Qt",
        "c:\\qt\\"
        // clang-format on
    };

    const std::vector<std::string> envVars = {
        // clang-format off
        "QTDIR",
        "QT_DIR",
        "QT6DIR"
        // clang-format on
    };

    for (const auto &envVar : envVars) {
        auto envValue = safeGetEnv(envVar.c_str());
        if (!envValue.empty()) {
            knownLocations.push_back(std::filesystem::path(envValue));
        }
    }

    auto homeDirEnv = safeGetEnv(HOME_DIR_ENV);
    if (!homeDirEnv.empty()) {
        auto homedir = std::filesystem::path(homeDirEnv) / "qt";
        knownLocations.push_back(homedir);
    }

    auto detected = std::vector<ExtraPath>();
    auto path_env = safeGetEnv("PATH");
    auto ss = std::stringstream(path_env);
    auto dir = std::string();
    while (std::getline(ss, dir, ENV_SEPARATOR)) {
        auto full_path = std::filesystem::path(dir) / (std::string("qmake") + BINARY_EXT);
        if (!std::filesystem::exists(full_path)) {
            continue;
        }
        if (isCompilerAlreadyFound(detected, dir)) {
            continue;
        }
        auto extraPath = ExtraPath();

        extraPath.name = std::string("Qt - ") + dir;
        extraPath.compiler_path = dir;
        if (unix_target) {
            extraPath.comment = "# qt installation";
            extraPath.command = "export QTDIR=%1";
            extraPath.command += "\n";
            extraPath.command += "export QT_DIR=%1";
        } else {
            extraPath.comment = "@rem qt installation";
            extraPath.command = "SET QTDIR=%1";
            extraPath.command += "\n";
            extraPath.command += "SET QT_DIR=%1";
        }

        replaceAll(extraPath.command, "%1", dir);
        replaceAll(extraPath.comment, "%1", dir);
        detected.push_back(extraPath);
    }

    for (const auto &root : knownLocations) {
        if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
            continue;
        }
        for (const auto &entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_directory()) {
                continue;
            }
            if (!isValidQtInstallation(entry.path())) {
                continue;
            }

            auto extraPath = ExtraPath();
            auto relativePath = std::filesystem::relative(entry.path(), root);
            extraPath.name = std::string("Qt - ") + relativePath.string();
            extraPath.compiler_path = entry.path().string();
            if (unix_target) {
                extraPath.comment = "# qt installation";
                extraPath.command = "export QTDIR=%1";
                extraPath.command += "\n";
                extraPath.command += "export QT_DIR=%1";
                extraPath.command += "\n";
                extraPath.command += "export PATH=$QTDIR/bin:$PATH";
            } else {
                extraPath.comment = "@rem qt installation";
                extraPath.command = "SET QTDIR=%1";
                extraPath.command += "\n";
                extraPath.command += "SET QT_DIR=%1";
                extraPath.command += "\n";
                extraPath.command += "SET PATH=%QTDIR%\\bin:%PATH%";
            }
            replaceAll(extraPath.command, "%1", entry.path().string());
            replaceAll(extraPath.comment, "%1", entry.path().string());
            detected.push_back(extraPath);
        }
    }
    return detected;
}

auto findCompilerTools(bool /*unix_target*/) -> std::vector<ExtraPath> {
    auto detected = std::vector<ExtraPath>();

#if defined(_WIN32)
    findCompilerToolsWindows(detected);
#endif

    return detected;
}

void generateKitFiles(const std::filesystem::path &path, const std::vector<ExtraPath> &tools,
                      const std::vector<ExtraPath> &compilers,
                      const std::vector<ExtraPath> &qtInstalls, bool unix_target) {
    auto kitNumber = 1;
    auto SCRIPT_EXTENSION = unix_target ? SCRIPT_EXTENSION_UNIX : SCRIPT_EXTENSION_WIN32;
    auto SCRIPT_HEADER = unix_target ? SCRIPT_HEADER_UNIX : SCRIPT_HEADER_WIN32;
    auto SCRIPT_SUFFIX = unix_target ? SCRIPT_SUFFIX_UNIX : SCRIPT_SUFFIX_WIN32;
    auto compilersSize = std::max<size_t>(1, compilers.size());
    auto qtinstallSize = std::max<size_t>(1, qtInstalls.size());
    for (auto i = 0ul; i < compilersSize; ++i) {
        for (auto j = 0ul; j < qtinstallSize; ++j) {
            auto cc = compilers.empty() ? ExtraPath() : compilers[i];
            auto qtInst = qtInstalls.empty() ? ExtraPath() : qtInstalls[j];

            auto scriptName = std::string("qtedit-kit-")
                                  .append(std::to_string(kitNumber))
                                  .append(SCRIPT_EXTENSION);
            auto scriptPath = path / scriptName;
            std::ofstream scriptFile(scriptPath.string());
            if (!scriptFile) {
                std::cerr << "Error: Could not create script file at " << scriptPath << std::endl;
                continue;
            }

            auto scriptDisplay = std::string(SCRIPT_HEADER);
            replaceAll(scriptDisplay, "@@NAME@@", cc.name + ", " + qtInst.name + " *");

            scriptFile << scriptDisplay;
            for (auto &tt : tools) {
                scriptFile << tt.comment;
                scriptFile << "\n";
                scriptFile << tt.command;
                scriptFile << "\n";
            }

            scriptFile << "\n";
            if (!cc.command.empty()) {
                scriptFile << cc.comment;
                scriptFile << "\n";
                scriptFile << cc.command;
                scriptFile << "\n";
                scriptFile << "\n";
            }

            if (!qtInst.command.empty()) {
                scriptFile << qtInst.comment;
                scriptFile << "\n";
                scriptFile << qtInst.command;
                scriptFile << "\n";
                scriptFile << "\n";
            }
            scriptFile << SCRIPT_SUFFIX;
#if defined(__unix__)
            auto c_path = scriptPath.c_str();
            if (chmod(c_path, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
                std::perror("chmod failed");
            }
#endif
            kitNumber++;
        }
    }
}

} // namespace KitDetector
