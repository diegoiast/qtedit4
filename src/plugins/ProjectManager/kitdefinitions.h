/**
 * \file kitdefinitions.h
 * \brief Declaration of kit definitions
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License MIT
 */

#pragma once

#include <string>
#include <vector>

struct KitDefinition {
    std::string filePath;
    std::string name;
    std::string author;

    KitDefinition(const std::string &filePath = "", const std::string &name = "",
                  const std::string &author = "")
        : filePath(filePath), name(name), author(author) {}
};

auto findKitDefinitions(const std::string_view directoryPath = {}) -> std::vector<KitDefinition>;
