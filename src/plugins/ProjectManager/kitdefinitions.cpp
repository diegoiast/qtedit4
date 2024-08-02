#include "kitdefinitions.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

static auto getExtensionPrefixMap() -> const std::unordered_map<std::string, std::string_view> & {
    static const std::unordered_map<std::string, std::string_view> EXTENSION_PREFIX_MAP = {
#ifdef _WIN32
        {".bat", "rem @@ "}, {".ps1", "# @@ "}
#else
        {".sh", "# @@ "}
#endif
    };
    return EXTENSION_PREFIX_MAP;
}

static auto trim(std::string_view str) -> std::string {
    auto start = str.find_first_not_of(" \t");
    if (start == std::string_view::npos) {
        return {};
    }
    auto end = str.find_last_not_of(" \t");
    return std::string(str.substr(start, end - start + 1));
}

static auto parseKeyValue(std::string_view line) -> std::pair<std::string, std::string> {
    auto pos = line.find('=');
    if (pos == std::string_view::npos) {
        return {"", ""};
    }
    auto key = trim(line.substr(0, pos));
    auto value = trim(line.substr(pos + 1));
    return {key, value};
}

static auto readFile(const std::filesystem::path &filePath, std::string_view prefix)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> keyValueMap;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filePath << std::endl;
        return keyValueMap;
    }

    std::string line;
    auto prefixLength = prefix.length();
    while (std::getline(file, line)) {
        auto trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine.substr(0, prefixLength) != prefix) {
            continue;
        }
        auto remaining = trimmedLine.substr(prefixLength);
        auto [key, value] = parseKeyValue(remaining);
        keyValueMap[std::move(key)] = std::move(value);
    }
    return keyValueMap;
}

auto findKitDefinitions(const std::string_view directoryPath) -> std::vector<KitDefinition> {
    if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
        return {};
    }

    auto fileInfoList = std::vector<KitDefinition>();
    for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto validExtensions = getExtensionPrefixMap();
        auto extension = entry.path().extension().string();
        auto it = validExtensions.find(extension);
        if (it == validExtensions.end()) {
            continue;
        }

        auto prefix = it->second;
        auto keyValueMap = readFile(entry.path(), prefix);
        if (!keyValueMap.empty()) {
            auto name = keyValueMap["name"];
            auto author = keyValueMap["author"];
            KitDefinition fileInfo(entry.path().string(), std::move(name), std::move(author));
            fileInfoList.push_back(std::move(fileInfo));
        }
    }
    return fileInfoList;
}

/*
// Uncomment for testing

int main() {
    std::string directoryPath = ".";

    auto fileInfoList = findKitDefinitions(directoryPath);

    for (const auto &info : fileInfoList) {
        std::cout << "File Path: " << info.filePath << std::endl;
        std::cout << "Name: " << info.name << std::endl;
        std::cout << "Author: " << info.author << std::endl;
    }

    return 0;
}
*/
