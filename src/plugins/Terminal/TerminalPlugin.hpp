/**
 * \file TerminalPlugin.hpp
 * \brief Definition of the Terminal plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 */

#pragma once

#include "iplugin.h"
#include "KodoTerm/KodoTermConfig.hpp"


class QDockWidget;
class QAction;
class KodoTerm;

class TerminalPlugin : public IPlugin {
    struct Config {
        CONFIG_DEFINE(Font, QString)
        CONFIG_DEFINE(ThemeFile, QString)
        CONFIG_DEFINE(ThemeFileChoose, QString)
        CONFIG_DEFINE(PromptPreview, QString)
        CONFIG_DEFINE(DoubleClick, bool)
        CONFIG_DEFINE(TrippleClickClick, bool)
        CONFIG_DEFINE(CopyOnSelect, bool)
        CONFIG_DEFINE(PasteOnMiddleClick, bool)
        qmdiPluginConfig *config;
    };
    Config &getConfig() {
        static Config configObject{&this->config};
        return configObject;
    }

    Q_OBJECT
  public:
    TerminalPlugin();
    ~TerminalPlugin();

    // IPlugin interface
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;
    virtual void loadConfig(QSettings &settings) override;

  private:
    QDockWidget *terminalDock = nullptr;
    QAction *toggleTerminal;
    KodoTerm *console = nullptr;
    TerminalTheme theme;
};
