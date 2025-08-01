#pragma once

#include "iplugin.h"

/**
 * \file SplitTabsPlugin.hpp
 * \brief Definition of the SplitTabsPlugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 */

#pragma once

class TextEditorPlugin;
class qmdiSplitTab;

class SplitTabsPlugin : public IPlugin {

    struct Config {
        CONFIG_DEFINE(SplitSizes, QString)
        CONFIG_DEFINE(SplitCount, QString)
        qmdiPluginConfig *config;
    };
    Config &getConfig();

  public:
    SplitTabsPlugin(TextEditorPlugin *p);
    ~SplitTabsPlugin();

    virtual void loadConfig(QSettings &settings) override;
    virtual void saveConfig(QSettings &settings) override;
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;

  private:
    TextEditorPlugin *textEditorPlugin = nullptr;
    qmdiSplitTab *split = nullptr;
};
