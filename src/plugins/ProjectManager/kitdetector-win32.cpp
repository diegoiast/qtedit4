#include "kitdetector.h"
#include <filesystem>
#include <shlobj.h>
#include <windows.h>

constexpr auto PLATFORM_PATH_DELIMITER = ';';


static auto wstringToString(const std::wstring& wstr) -> std::string {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &str[0], size_needed, nullptr, nullptr);
    return str;
}

static auto checkVisualStudioVersion(PWSTR basePath,
                                     const std::wstring &version, KitDetector::ExtraPath &extraPath)
    -> bool {
    std::wstring basePathW(basePath);
    std::filesystem::path versionPath = std::filesystem::path(basePathW)/ "Microsoft Visual Studio" / version;
    if (std::filesystem::exists(versionPath)) {
        extraPath.name = wstringToString(version);
        extraPath.compiler_path = wstringToString(versionPath);
        extraPath.comment = "@rem VS " + wstringToString(version);
        extraPath.command = "call %1\\vcallvars.bat";
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
