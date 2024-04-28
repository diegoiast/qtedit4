#pragma once

/**
 * \file qexeditor.h
 * \brief Definition of the extended text editor class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see MainWindow
 */

#include <QTextEdit>

#include "qmdiclient.h"

class QString;
class QTextEdit;
class QToolBar;
class QAction;
class QTextCodec;

class QexTextEdit : public QTextEdit, public qmdiClient {
    Q_OBJECT
  public:
    QexTextEdit(QString file = QString(), bool singleToolbar = false, QWidget *parent = 0);
    ~QexTextEdit();

    bool canCloseClient();
    QString mdiClientFileName();

    void initInterface(bool singleToolbar = false);
    bool openFile(QString newFile);
    bool saveFile(QString newFile);

  public slots:
    bool fileSave();
    bool fileSaveAs();
    bool fileClose();
    void helpShowHelp();

  protected:
    QAction *actionSave;
    QAction *actionClose;

    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCopy;
    QAction *actionCut;
    QAction *actionPaste;
    QAction *actionFind;
    QAction *actiohAskHelp;

    QString fileName;
};
