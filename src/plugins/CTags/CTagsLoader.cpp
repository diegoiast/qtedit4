#include "CTagsLoader.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

CTagsLoader::CTagsLoader(const std::string &ctagsBinary) : ctagsBinary(ctagsBinary) {}

bool CTagsLoader::loadFile(const std::string &file) {
    filename = file;
    tags.clear();
    return load();
}

bool CTagsLoader::scanFiles(const std::vector<std::string> &files) {
    std::string command = ctagsBinary + " -x ";
    for (const auto &f : files) {
        command += "\"" + f + "\" ";
    }
    std::string ctagsOutput = runCommand(command);
    return parseCtagsOutput(ctagsOutput);
}

bool CTagsLoader::scanFiles(const std::string &ctagsFileName,
                            const std::vector<std::string> &files) {
    std::string command = ctagsBinary + " -f \"" + ctagsFileName + "\" ";
    for (const auto &f : files) {
        command += "\"" + f + "\" ";
    }
    if (system(command.c_str()) != 0) {
        std::cerr << "Error: Failed to generate ctags file." << std::endl;
        return false;
    }
    bool result = loadFile(ctagsFileName);
    std::filesystem::remove(ctagsFileName);
    return result;
}

bool CTagsLoader::scanDirs(const std::string &dir) {
    std::string command = ctagsBinary + " -x -R \"" + dir + "\"";
    std::string ctagsOutput = runCommand(command);
    return parseCtagsOutput(ctagsOutput);
}

bool CTagsLoader::scanDirs(const std::string &ctagsFileName, const std::string &dir) {
    std::string logFile = ctagsFileName + ".log";
    std::string command = ctagsBinary + " -R -f \"" + ctagsFileName + "\" \"" + dir + "\"";

    // std::cerr << command;
    std::thread([=, this]() {
#if defined(_WIN32) || defined(_WIN64)
        std::string fullCommand = "cmd /C \"" + command + " > \"" + logFile + "\" 2>&1\"";
        STARTUPINFOA si = {sizeof(si), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessA(NULL, const_cast<LPSTR>(fullCommand.c_str()), NULL, NULL, FALSE,
                           CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cerr << "Error: Failed to run the command." << std::endl;
        }

#else
        std::string fullCommand = command + " > " + logFile + " 2>&1";
        if (system(fullCommand.c_str()) != 0) {
            std::cerr << "Error: Failed to generate ctags file." << std::endl;
        }
#endif
        loadFile(ctagsFileName);
    }).detach();

    return true;
}

CTagsLoader::TagListRef CTagsLoader::findTags(const std::string &symbolName,
                                              bool exactMatch) const {
    std::vector<std::reference_wrapper<const CTag>> foundTags;
    std::string symbolLower = symbolName;
    std::transform(symbolLower.begin(), symbolLower.end(), symbolLower.begin(), ::tolower);

    if (symbolName.length() >= 3) {
        if (exactMatch) {
            for (const auto &tag : tags) {
                if (tag.name == symbolName) {
                    foundTags.emplace_back(tag);
                }
            }
        } else {
            for (const auto &tag : tags) {
                auto tagLower = std::string(tag.name);
                std::transform(tagLower.begin(), tagLower.end(), tagLower.begin(), ::tolower);
                if (tagLower.starts_with(symbolLower)) {
                    foundTags.emplace_back(tag);
                }
            }
        }
    }
    return foundTags;
}

void CTagsLoader::setCTagsBinary(const std::string &ctagsBinary) {
    this->ctagsBinary = ctagsBinary;
}

void CTagsLoader::clear() { tags.clear(); }

bool CTagsLoader::load() {
    using namespace std::chrono;
    auto start = steady_clock::now();

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "CTAGS: Error: Could not open file " << filename << std::endl;
        return false;
    }

    tags.clear();
    // tags.reserve(500000); // Adjust based on expected file size

    std::string line;
    size_t count = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '!') {
            continue;
        }

        tags.emplace_back(); // add a new empty CTag
        CTag &tag = tags.back();

        tag.originalLine = std::move(line); // store original line
        std::string_view sv(tag.originalLine);

        size_t tab1 = sv.find('\t');
        if (tab1 == std::string_view::npos) {
            tags.pop_back();
            continue;
        }

        size_t tab2 = sv.find('\t', tab1 + 1);
        if (tab2 == std::string_view::npos) {
            tags.pop_back();
            continue;
        }

        size_t tab3 = sv.find('\t', tab2 + 1);

        tag.name = sv.substr(0, tab1);
        tag.file = sv.substr(tab1 + 1, tab2 - tab1 - 1);
        tag.address = sv.substr(
            tab2 + 1, (tab3 == std::string_view::npos ? std::string_view::npos : tab3 - tab2 - 1));

        tag.fieldKey = TagFieldKey::Unknown;
        if (tab3 != std::string_view::npos) {
            size_t tab4 = sv.find('\t', tab3 + 1);
            std::string_view field =
                sv.substr(tab3 + 1, (tab4 == std::string_view::npos ? std::string_view::npos
                                                                    : tab4 - tab3 - 1));

            if (!field.empty()) {
                tag.fieldKey = mapCharToTagFieldKey(field[0]);
                if (tab4 != std::string_view::npos && tab4 + 1 < sv.size()) {
                    tag.fieldValue = sv.substr(tab4 + 1);
                }
            }
        }

        ++count;
    }

    file.close();

    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    std::cout << "CTAGS: Loaded " << count << " tags from " << filename << " in " << duration
              << " ms" << std::endl;

    return true;
}

bool CTagsLoader::parseCtagsOutput(const std::string &ctagsOutput) {
    std::stringstream ss(ctagsOutput);
    std::string line;
    while (std::getline(ss, line)) {
        std::stringstream lineStream(line);
        std::string tagName, type, lineNumberStr, tagFile, tagAddress;

        lineStream >> tagName >> type >> lineNumberStr >> tagFile >> std::ws;
        tagAddress = line.substr(lineStream.tellg());

        CTag newTag = {tagName, tagFile, tagAddress, TagFieldKey::Unknown, {}, {}};
        tags.push_back(newTag);
    }
    return true;
}

std::string CTagsLoader::runCommand(const std::string &command) {
    std::string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
    }
    return result;
}

TagFieldKey mapCharToTagFieldKey(char key) {
    switch (key) {
    // Classes and structures
    case 'c':
        return TagFieldKey::Class;
    case 's':
        return TagFieldKey::Struct;

    // Functions and methods
    case 'f':
        return TagFieldKey::Function;
    case 'm':
        return TagFieldKey::Method;

    // Variables and fields
    case 'v':
        return TagFieldKey::Variable;
    case 'g':
        return TagFieldKey::EnumName;
    case 'e':
        return TagFieldKey::EnumValue;

    // Namespaces and scope
    case 'n':
        return TagFieldKey::Namespace;
    case 'd':
        return TagFieldKey::Macro;

    // Type information
    case 't':
        return TagFieldKey::Type;
    case 'p':
        return TagFieldKey::Prototype;

    // Other metadata
    case 'F':
        return TagFieldKey::FileScope;
    case 'i':
        return TagFieldKey::Inheritance;
    case 'l':
        return TagFieldKey::Language;
    case 'k':
        return TagFieldKey::Kind;
    case 'r':
        return TagFieldKey::Regex;
    }
    return TagFieldKey::Unknown;
}

std::string tagFieldKeyToString(TagFieldKey key) {
    switch (key) {
    case TagFieldKey::Class:
        return "Class";
    case TagFieldKey::Struct:
        return "Struct";
    case TagFieldKey::Function:
        return "Function";
    case TagFieldKey::Method:
        return "Method";
    case TagFieldKey::Variable:
        return "Variable";
    case TagFieldKey::EnumName:
        return "EnumName";
    case TagFieldKey::EnumValue:
        return "EnumValue";
    case TagFieldKey::Namespace:
        return "Namespace";
    case TagFieldKey::Macro:
        return "Macro";
    case TagFieldKey::Type:
        return "Type";
    case TagFieldKey::Prototype:
        return "Prototype";
    case TagFieldKey::FileScope:
        return "FileScope";
    case TagFieldKey::Inheritance:
        return "Inheritance";
    case TagFieldKey::Language:
        return "Language";
    case TagFieldKey::Kind:
        return "Kind";
    case TagFieldKey::Regex:
        return "Regex";
    case TagFieldKey::Unknown:
        return "Unknown";
    }
    return "Invalid";
}
