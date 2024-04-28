#pragma once

/**
 * \file richtext_plg.h
 * \brief Definition of the RichTextPlugin class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see RichTextPlugin
 */

#include "iplugin.h"

class QAction;

class RichTextPlugin : public IPlugin {
    Q_OBJECT
  public:
    RichTextPlugin();
    ~RichTextPlugin();

    void showAbout();
    QWidget *getConfigDialog();
    QActionGroup *newFileActions();
    QStringList myExtensions();
    int canOpenFile(const QString fileName);
    bool canCloseClient();
    bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1);
    void getData();
    void setData();

  public slots:
    void fileNew();

  private:
    QAction *actionNew;
    QActionGroup *_newFileActions;

    bool makeBackups;
    bool showLineNumbers;
    bool makeCurrentLine;
    bool wordWrap;
    QFont editorFont;
    int endOfLine;
};
