/**
 * \file texteditor_plg
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

#include "iplugin.h"

class QsvColorDefFactory;

// clang-format off
#define CONFIG_DEFINE(key, type) \
    static constexpr auto key##Key = #key; \
    type get##key() const { \
        return config->getVariable<type>(key##Key); \
    } \
    void set##key(const type &value) { \
        config->setVariable<type>(key##Key, value); \
    }
// clang-format on

class TextEditorPlugin : public IPlugin {

    struct Config {
        CONFIG_DEFINE(TrimSpaces, bool)
        CONFIG_DEFINE(Margin, bool)
        CONFIG_DEFINE(WrapLines, bool);
        CONFIG_DEFINE(AutoReload, bool)
        CONFIG_DEFINE(ShowWhite, bool)
        CONFIG_DEFINE(ShowIndentations, bool)
        CONFIG_DEFINE(HighlightBrackets, bool)
        CONFIG_DEFINE(ShowLine, bool)
        CONFIG_DEFINE(MarginOffset, int)
        qmdiPluginConfig *config;
    };
    Config &getConfig() {
        static Config configObject{&this->config};
        return configObject;
    }

    Q_OBJECT
  public:
    TextEditorPlugin();
    ~TextEditorPlugin();

    void showAbout();
    QWidget *getConfigDialog();
    QActionGroup *newFileActions();
    QStringList myExtensions();
    int canOpenFile(const QString fileName);
    bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1);
    void navigateFile(qmdiClient *client, int x, int y, int z);
    void getData();
    void setData();
    void applySettings(qmdiClient *);

  public slots:
    void fileNew(QAction *);

  private:
    QActionGroup *myNewActions;
    QAction *actionNewFile;
    QAction *actionNewCPP;
    QAction *actionNewHeader;

    QsvColorDefFactory *editorColors;
};
