#include "kitdetector.h"
#include <filesystem>
#include <shlobj.h>
#include <windows.h>

constexpr auto PLATFORM_PATH_DELIMITER = ';';

static auto checkVisualStudioVersion(const std::filesystem::path &basePath,
                                     const std::string &version, KitDetector::ExtraPath &extraPath)
    -> bool {
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
