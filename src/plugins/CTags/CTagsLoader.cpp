#include "CTagsLoader.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

CTagsLoader::CTagsLoader(const std::string &ctagsBinary) : ctagsBinary(ctagsBinary) {}

bool CTagsLoader::loadFile(const std::string &file) {
    filename = file;
    // tags.clear();
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
    std::string command = ctagsBinary + " -R -f \"" + ctagsFileName + "\" \"" + dir + "\"";
    if (system(command.c_str()) != 0) {
        std::cerr << "Error: Failed to generate ctags file." << std::endl;
        return false;
    }
    bool result = loadFile(ctagsFileName);
    return result;
}

std::optional<CTag> CTagsLoader::findTag(const std::string &symbolName) const {
    auto it = std::find_if(tags.begin(), tags.end(),
                           [&symbolName](const CTag &tag) { return tag.tagName == symbolName; });

    if (it != tags.end()) {
        return *it;
    } else {
        return std::nullopt;
    }
}

void CTagsLoader::setCtagsBinary(const std::string &ctagsBinary) {
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
    std::ifstream file(tag.tagFile);
    if (!file.is_open()) {
        return;
    }

    int lineNumber = -1;
    std::regex pattern;
    bool patternSet = false;

    if (tag.tagAddress[0] == '/') {
        std::string patternStr = tag.tagAddress.substr(1, tag.tagAddress.length() - 2);
        try {
            pattern = std::regex(patternStr);
            patternSet = true;
        } catch (const std::regex_error &e) {
            return;
        }
    } else {
        try {
            lineNumber = std::stoi(tag.tagAddress);
        } catch (const std::invalid_argument &e) {
            return;
        }
    }

    std::string line;
    int currentLine = 1;
    while (std::getline(file, line)) {
        if (lineNumber != -1 && currentLine == lineNumber) {
            std::smatch match;
            if (std::regex_search(line, match, std::regex(tag.tagName))) {
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

TagFieldKey CTagsLoader::mapCharToTagFieldKey(char key) const {
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
    default:
        return TagFieldKey::Unknown;
    }
}