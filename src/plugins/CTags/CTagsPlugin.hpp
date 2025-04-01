/**
 * \file CTagsPLugin.hpp
 * \brief Definition of the help system  plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 * \see HelpPlugin
 */

#pragma once

#include "iplugin.h"

class CTagsLoader;
// #include "CTagsLoader.hpp"

class CTagsPlugin : public IPlugin {
    struct Config {
        CONFIG_DEFINE(CTagsBinary, QString)
        qmdiPluginConfig *config;
    };
    Config &getConfig();
    CTagsLoader *ctags;

  public:
    CTagsPlugin();
    ~CTagsPlugin();

    virtual int canHandleCommand(const QString &command, const CommandArgs &args) const override;
    virtual CommandArgs handleCommand(const QString &command, const CommandArgs &args) override;
};
