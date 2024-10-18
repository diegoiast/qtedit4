/**
 * \file texteditor_plg
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

namespace Qutepart {
class ThemeManager;
class Theme;
} // namespace Qutepart

#include "endlinestyle.h"
#include "iplugin.h"

class qmdiEditor;

class TextEditorPlugin : public IPlugin {
    struct Config {
        CONFIG_DEFINE(TrimSpaces, bool)
        CONFIG_DEFINE(SmartHome, bool)
        CONFIG_DEFINE(WrapLines, bool)
        CONFIG_DEFINE(AutoReload, bool)
        CONFIG_DEFINE(ShowWhite, bool)
        CONFIG_DEFINE(ShowIndentations, bool)
        CONFIG_DEFINE(HighlightBrackets, bool)
        CONFIG_DEFINE(Margin, bool)
        CONFIG_DEFINE(ShowLine, bool)
        CONFIG_DEFINE(MarginOffset, int)
        CONFIG_DEFINE(LineEndingSave, EndLineStyle)
        CONFIG_DEFINE(Font, QString)
        CONFIG_DEFINE(Theme, QString)
        qmdiPluginConfig *config;
    };
    Config &getConfig() {
        static Config configObject{&this->config};
        return configObject;
    }
    Qutepart::ThemeManager *themeManager;
    Qutepart::Theme *theme = nullptr;

    Q_OBJECT
  public:
    TextEditorPlugin();
    ~TextEditorPlugin();

    virtual void on_client_merged(qmdiHost *host) override;
    virtual void showAbout() override;
    virtual void loadConfig(QSettings &settings) override;

    QStringList myExtensions() override;
    int canOpenFile(const QString fileName) override;
    bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1) override;
    void navigateFile(qmdiClient *client, int x, int y, int z) override;
    void applySettings(qmdiEditor *editor);

  public slots:
    virtual void configurationHasBeenModified() override;
    void fileNew();

  private:
    QAction *chooseTheme;

    // TODO - this needs to be a local variable, or at lest the nested
    //        hack of functions needs to be removed , see chooseTheme, &QAction::triggered
    bool newThemeSelected = false;
};
