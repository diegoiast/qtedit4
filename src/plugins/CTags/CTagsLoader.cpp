#include "CTagsLoader.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
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
        std::string fullCommand =
            command + " > " + logFile + " 2>&1 &"; // The '&' sends it to the background
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
        for (const auto &tag : tags) {
            if (exactMatch) {
                if (tag.name == symbolName) {
                    foundTags.emplace_back(tag);
                }
            } else {
                std::string tagLower = tag.name;
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
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '!') {
            continue;
        }

        std::stringstream ss(line);
        std::string tagName, tagFile, tagAddress, field;
        TagFieldKey tagField = TagFieldKey::Unknown;
        std::string tagFieldValue;

        std::getline(ss, tagName, '\t');
        std::getline(ss, tagFile, '\t');
        std::getline(ss, tagAddress, '\t');

        if (std::getline(ss, field, '\t')) {
            tagField = mapCharToTagFieldKey(field[0]);
            std::getline(ss, tagFieldValue, '\t');
        }

        CTag newTag = {tagName, tagFile, tagAddress, tagField, tagFieldValue};
        // calculateLineColumn(newTag);
        tags.push_back(newTag);
    }

    file.close();
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

        CTag newTag = {tagName, tagFile, tagAddress, TagFieldKey::Unknown, {}};
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
