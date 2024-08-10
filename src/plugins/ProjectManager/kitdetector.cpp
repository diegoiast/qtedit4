/**
 * \file kitdefinitions.cpp
 * \brief Implementation of kit detector
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License MIT
 */

#include "kitdetector.h"
#include <fstream>
#include <iostream>

#if defined(__unix__)
#include "kitdetector-unix.cpp"
#include <sys/stat.h>
constexpr auto HOME_DIR_ENV = "HOME";
constexpr auto BINARY_EXT = "";
#endif

#if defined(_WIN32)
#include "kitdetector-win32.cpp"
constexpr auto HOME_DIR_ENV = "USERPROFILE";
constexpr auto BINARY_EXT = ".exe";
#endif

constexpr auto SCRIPT_EXTENSION_UNIX = ".sh";
constexpr auto SCRIPT_HEADER_UNIX = R"(#! /bin/sh

# This is a kit definition for qtedit4. All your tasks
# will run trought this file.

# available enritonment variables:
# ${source_directory} - where the source being executed is saved
# ${build_directory}  - where code should be compile into
# ${run_directory}    - where this taks should be run, as defined in
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
rem will run trought this file.

rem available enritonment variables:
rem %source_directory% - where the source being executed is saved
rem %build_directory%  - where code should be compile into
rem %run_directory%    - where this taks should be run, as defined in 
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

auto replaceAll(std::string &str, const std::string &from, const std::string &to) -> std::string & {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

auto findCompilers() -> std::vector<ExtraPath> {
    auto detected = std::vector<ExtraPath>();

#if defined(__unix__)
    findCompilersUnix(detected);
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

auto findQtVersions(bool unix_target) -> std::vector<ExtraPath> {
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
                    auto relativePath = std::filesystem::relative(entry.path(), root);

                    extraPath.name = std::string("Qt") + relativePath.string();
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
        }
    }

    return detected;
}

auto findCompilerTools() -> std::vector<ExtraPath> {
    auto detected = std::vector<ExtraPath>();
#if defined(__unix__)
    findCompilerToolsUnix(detected);
#endif

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

    for (auto &cc : compilers) {
        for (auto &qtInst : qtInstalls) {
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
            replaceAll(scriptDisplay, "@@NAME@@", "AUTO: " + cc.name + " " + qtInst.name);

            scriptFile << scriptDisplay;
            for (auto &tt : tools) {
                scriptFile << tt.comment;
                scriptFile << "\n";
                scriptFile << tt.command;
                scriptFile << "\n";
            }
            scriptFile << "\n";
            scriptFile << cc.comment;
            scriptFile << "\n";
            scriptFile << cc.command;
            scriptFile << "\n";
            scriptFile << "\n";
            scriptFile << qtInst.comment;
            scriptFile << "\n";
            scriptFile << qtInst.command;
            scriptFile << "\n";
            scriptFile << "\n";
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
