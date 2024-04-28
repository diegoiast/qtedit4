/**
 * \file qexeditor.cpp
 * \brief Definition of the extended text editor class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see MainWindow
 */

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QTextStream>

#include "qexeditor.h"

/**
 * \class QexTextEdit
 * \brief a small text editor based on QTextEdit
 *
 * This class demostrates how to make a small text editor
 * using QTextEdit from the Qt library and qmdilib.
 *
 * The class contains the basic cut/copy/paste/undo/redo actions,
 * as well as close. It full integrates into the host
 * application, by adding context menus and toolbars when the
 * widget is selected on a qmdiTabWidget.
 *
 * The class is a qmdiClient and all the menus and toolbars are defined
 * using that interface.
 *
 * \see qmdiClient
 */

QexTextEdit::QexTextEdit(QString file, bool singleToolbar, QWidget *parent) : QTextEdit(parent) {
    QFont font;
    font.setFamily("Courier New");
    font.setPointSize(10);
    font.setFixedPitch(true);
    setFont(font);
    setAcceptRichText(false);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setFrameStyle(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);

    actionSave = new QAction(QIcon(":images/save.png"), tr("&Save"), this);
    actionClose = new QAction(QIcon(":images/fileclose.png"), tr("&Close"), this);
    actionUndo = new QAction(QIcon(":images/redo.png"), tr("&Redo"), this);
    actionRedo = new QAction(QIcon(":images/undo.png"), tr("&Undo"), this);
    actionCopy = new QAction(QIcon(":images/copy.png"), tr("&Copy"), this);
    actionCut = new QAction(QIcon(":images/cut.png"), tr("&cut"), this);
    actionPaste = new QAction(QIcon(":images/paste.png"), tr("&Paste"), this);
    actionFind = new QAction(QIcon(":images/find.png"), tr("&Find"), this);
    actiohAskHelp = new QAction(tr("Help about this calss"), this);

    connect(this, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    // 	connect( this, SIGNAL(pasteAvailable(bool)), actionPaste,
    // SLOT(setEnabled(bool)) );
    connect(this, SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));

    connect(actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), this, SLOT(redo()));
    connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(actionCut, SIGNAL(triggered()), this, SLOT(cut()));
    connect(actionPaste, SIGNAL(triggered()), this, SLOT(paste()));
    connect(actionClose, SIGNAL(triggered()), this, SLOT(fileClose()));
    connect(actiohAskHelp, SIGNAL(triggered()), this, SLOT(helpShowHelp()));

    actionUndo->setEnabled(false);
    actionRedo->setEnabled(false);
    actionCopy->setEnabled(false);
    actionCut->setEnabled(false);
    actionPaste->setEnabled(false);
    actionClose->setToolTip("Closing from the widget itself");

    initInterface(singleToolbar);
    openFile(file);
}

QexTextEdit::~QexTextEdit() {
    // TODO
}

bool QexTextEdit::canCloseClient() {
    if (!document()->isModified())
        return true;

    // ask for saving
    int ret = QMessageBox::warning(this, tr("Application"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Yes | QMessageBox::Default, QMessageBox::No,
                                   QMessageBox::Cancel | QMessageBox::Escape);

    if (ret == QMessageBox::Yes)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;

    // shut up GCC warnings
    return true;
}

QString QexTextEdit::mdiClientFileName() { return fileName; }

void QexTextEdit::initInterface(bool singleToolbar) {
    QString toolbarFile = singleToolbar ? "main" : "File";
    QString toolbarEdit = singleToolbar ? "main" : "Edit operations";

    // define the menus for this widget
    menus["&File"]->addAction(actionSave);
    menus["&File"]->addAction(actionClose);
    menus["&Edit"]->addAction(actionUndo);
    menus["&Edit"]->addAction(actionRedo);
    menus["&Edit"]->addSeparator();
    menus["&Edit"]->addAction(actionCopy);
    menus["&Edit"]->addAction(actionCut);
    menus["&Edit"]->addAction(actionPaste);
    menus["&Search"]->addAction(actionFind);

    // define the toolbars for this widget
    if (singleToolbar)
        toolbars[toolbarFile]->addSeparator();

    toolbars[toolbarFile]->breakAfter = true;
    toolbars[toolbarFile]->addAction(actionSave);
    toolbars[toolbarFile]->addAction(actionClose);

    if (singleToolbar)
        toolbars[toolbarEdit]->addSeparator();

    toolbars[toolbarEdit]->addAction(actionCopy);
    toolbars[toolbarEdit]->addAction(actionCut);
    toolbars[toolbarEdit]->addAction(actionPaste);
    toolbars[toolbarEdit]->addSeparator();
    toolbars[toolbarEdit]->addAction(actionUndo);
    toolbars[toolbarEdit]->addAction(actionRedo);
}

bool QexTextEdit::openFile(QString newFile) {
    fileName = newFile;     // the full path of the loaded file
    setObjectName(newFile); // set the object name to the file name,

    // the name of the object for it's mdi server
    // is the file name alone, without the directory
    int i = newFile.lastIndexOf('/');
    QString s;
    if (i != -1)
        s = newFile.mid(i + 1);
    else {
        i = newFile.lastIndexOf('\\');
        if (i != -1)
            s = newFile.mid(i + 1);
        else
            s = newFile;
    }
    mdiClientName = s;

    if (newFile.isEmpty())
        return true;

    QFile f(fileName);

    if (!f.open(QIODevice::ReadOnly))
        return false;

    QTextStream t(&f);
    setPlainText(t.readAll());
    f.close();

    return true;
}

bool QexTextEdit::saveFile(QString newFile) {
    QFile f(newFile);

    if (!f.open(QIODevice::WriteOnly))
        return false;

    QTextStream t(&f);
    t << toPlainText();
    f.close();

    return true;
}

bool QexTextEdit::fileSave() {
    if (fileName.isEmpty())
        return fileSaveAs();

    return saveFile(fileName);
}

bool QexTextEdit::fileSaveAs() {

    QString s = QFileDialog::getSaveFileName(NULL, "Choose a filename to save under", QString(),
                                             "Sources (*.c *.cpp *.cxx *.h *.hpp *.hxx *.inc);;"
                                             "Headers (*.h *.hpp *.hxx *.inc);;"
                                             "Qt project (*.pro *.pri);;"
                                             "All files (*.*)");

    if (s.isEmpty())
        return false;

    fileName = s;
    return saveFile(fileName);
}

bool QexTextEdit::fileClose() { return closeClient(); }

void QexTextEdit::helpShowHelp() {
    if (!mdiServer)
        return;

    // 	mdiServer->
}
