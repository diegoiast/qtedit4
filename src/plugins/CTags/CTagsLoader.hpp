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
    FileScope,
    Variable,
    Type,
    Prototype,
    Scope,
    Inheritance,
    Language,
    Regex,
    Kind,
    Unknown
};

struct CTag {
    std::string tagName;
    std::string tagFile;
    std::string tagAddress;
    int lineNumber = -1;
    int columnNumber = -1;
    std::map<TagFieldKey, std::string> tagFields;
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

    std::optional<CTag> findTag(const std::string &symbolName) const;

  private:
    bool load();
    bool parseCtagsOutput(const std::string &ctagsOutput);
    std::string runCommand(const std::string &command);
    void calculateLineColumn(CTag &tag) const;
    TagFieldKey mapCharToTagFieldKey(char key) const;

    std::string filename;
    std::vector<CTag> tags;
    std::string ctagsBinary;
};
