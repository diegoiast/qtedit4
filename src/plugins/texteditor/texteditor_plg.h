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

class TextEditorPlugin : public IPlugin {

    struct ConfigNames {
        static constexpr auto TrimSpaces = "TrimSpaces";
        static constexpr auto Margin = "Margin";
        static constexpr auto WrapLines = "WrapLines";
        static constexpr auto AutoReload = "AutoReload";
        static constexpr auto ShowWhiteSpace = "ShowWhiteSpace";
        static constexpr auto ShowIndentations = "ShowIndentations";
        static constexpr auto HighlightBrackets = "HighlightBrackets";
        static constexpr auto ShowLineNumbers = "ShowLineNumbers";
        static constexpr auto MarginOffset = "MarginOffset";
    };

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

  public slots:
    void fileNew(QAction *);

  private:
    QActionGroup *myNewActions;
    QAction *actionNewFile;
    QAction *actionNewCPP;
    QAction *actionNewHeader;

    QsvColorDefFactory *editorColors;
};
