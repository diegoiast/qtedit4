#pragma once
/**
 * \file editorplugin.h
 * \brief Definition of the EditorPlugin class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see EditorPlugin
 */

#include "iplugin.h"
#include "ui_editor_cfg.h"

class QAction;

class EditorPlugin : public IPlugin {
    Q_OBJECT
  public:
    EditorPlugin();
    ~EditorPlugin();

    void showAbout();
    QWidget *getConfigDialog();
    QActionGroup *newFileActions();
    QStringList myExtensions();
    int canOpenFile(const QString fileName);
    bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1);
    void getData();
    void setData();
    void loadConfig(QSettings &settings);
    void saveConfig(QSettings &settings);

  public slots:
    void fileNew();

  private:
    QAction *actionNew;
    Ui::EditorCfgUI ui;
    QWidget *configUI;
    QActionGroup *_newFileActions;

    bool makeBackups;
    bool showLineNumbers;
    bool makeCurrentLine;
    bool wordWrap;
    QFont editorFont;
    int endOfLine;
};
