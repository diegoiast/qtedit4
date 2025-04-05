#include "CTagsLoader.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
    for (const auto &tag : tags) {
        if (exactMatch) {
            if (tag.name == symbolName) {
                foundTags.emplace_back(tag);
            }
        } else {
            if (tag.name.rfind(symbolName, 0) == 0) {
                foundTags.emplace_back(tag);
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
        std::map<TagFieldKey, std::string> tagFields;

        std::getline(ss, tagName, '\t');
        std::getline(ss, tagFile, '\t');
        std::getline(ss, tagAddress, '\t');

        while (std::getline(ss, field, '\t')) {
            if (field.length() >= 2 && field[0] == 2) {
                char key = field[1];
                std::string value = field.substr(2);
                TagFieldKey fieldKey = mapCharToTagFieldKey(key);
                tagFields[fieldKey] = value;
            }
        }

        CTag newTag = {tagName, tagFile, tagAddress, -1, -1, tagFields};
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
        std::map<TagFieldKey, std::string> tagFields;

        lineStream >> tagName >> type >> lineNumberStr >> tagFile >> std::ws;
        tagAddress = line.substr(lineStream.tellg());

        CTag newTag = {tagName, tagFile, tagAddress, std::stoi(lineNumberStr), 1, tagFields};
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

void CTagsLoader::calculateLineColumn(CTag &tag) const {
    std::ifstream file(tag.file);
    if (!file.is_open()) {
        return;
    }

    int lineNumber = -1;
    std::regex pattern;
    bool patternSet = false;

    if (tag.address[0] == '/') {
        std::string patternStr = tag.address.substr(1, tag.address.length() - 2);
        try {
            pattern = std::regex(patternStr);
            patternSet = true;
        } catch (const std::regex_error &e) {
            return;
        }
    } else {
        try {
            lineNumber = std::stoi(tag.address);
        } catch (const std::invalid_argument &e) {
            return;
        }
    }

    std::string line;
    int currentLine = 1;
    while (std::getline(file, line)) {
        if (lineNumber != -1 && currentLine == lineNumber) {
            std::smatch match;
            if (std::regex_search(line, match, std::regex(tag.name))) {
                tag.lineNumber = lineNumber;
                tag.columnNumber = static_cast<int>(match.position()) + 1;
                return;
            }
            return;
        } else if (patternSet) {
            std::smatch match;
            if (std::regex_search(line, match, pattern)) {
                tag.lineNumber = currentLine;
                tag.columnNumber = static_cast<int>(match.position()) + 1;
                return;
            }
        }
        currentLine++;
    }
}

TagFieldKey mapCharToTagFieldKey(char key) {
    switch (key) {
    case 'f':
        return TagFieldKey::FileScope;
    case 'v':
        return TagFieldKey::Variable;
    case 't':
        return TagFieldKey::Type;
    case 'p':
        return TagFieldKey::Prototype;
    case 's':
        return TagFieldKey::Scope;
    case 'i':
        return TagFieldKey::Inheritance;
    case 'l':
        return TagFieldKey::Language;
    case 'r':
        return TagFieldKey::Regex;
    case 'k':
        return TagFieldKey::Kind;
    }
    return TagFieldKey::Unknown;
}

std::string tagFieldKeyToString(TagFieldKey key) {
    switch (key) {
    case TagFieldKey::FileScope:
        return "FileScope";
    case TagFieldKey::Variable:
        return "Variable";
    case TagFieldKey::Type:
        return "Type";
    case TagFieldKey::Prototype:
        return "Prototype";
    case TagFieldKey::Scope:
        return "Scope";
    case TagFieldKey::Inheritance:
        return "Inheritance";
    case TagFieldKey::Language:
        return "Language";
    case TagFieldKey::Regex:
        return "Regex";
    case TagFieldKey::Kind:
        return "Kind";
    case TagFieldKey::Unknown:
        return "Unknown";
    }
    return "Invalid";
}
