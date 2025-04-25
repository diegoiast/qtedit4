#pragma once

// MIT License
//
// Copyright (c) [Year] [Your Name/Organization]
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <map>
#include <optional>
#include <string>
#include <vector>

enum class TagFieldKey {
    Unknown,
    // Classes and structures
    Class,
    Struct,

    // Functions and methods
    Function,
    Method,

    // Variables and fields
    Variable,
    EnumName,
    EnumValue,

    // Namespaces and scope
    Namespace,
    Macro,

    // Type information
    Type,
    Prototype,

    // Other metadata
    FileScope,
    Inheritance,
    Language,
    Kind,
    Regex
};

struct CTag {
    std::string_view name;
    std::string_view file;
    std::string_view address;
    TagFieldKey fieldKey;
    std::string_view fieldValue;

    // Owns the memory
    std::string originalLine;
};

class CTagsLoader {
  public:
    CTagsLoader(const std::string &ctagsBinary = "ctags");
    void setCTagsBinary(const std::string &ctagsBinary);

    void clear();
    bool loadFile(const std::string &file);
    bool scanFiles(const std::vector<std::string> &files);
    bool scanFiles(const std::string &ctagsFileName, const std::vector<std::string> &files);
    bool scanDirs(const std::string &dir);
    bool scanDirs(const std::string &ctagsFileName, const std::string &dir);

    using TagListRef = std::vector<std::reference_wrapper<const CTag>>;
    TagListRef findTags(const std::string &symbolName, bool exactMatch) const;

  private:
    bool load();
    bool parseCtagsOutput(const std::string &ctagsOutput);
    std::string runCommand(const std::string &command);

    std::string filename;
    std::vector<CTag> tags;
    std::string ctagsBinary;
};

std::string tagFieldKeyToString(TagFieldKey key);
TagFieldKey mapCharToTagFieldKey(char key);
