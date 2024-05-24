/**
 * \file qmdieditor
 * \brief Implementation of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#include "qmdieditor.h"
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <qsvte/qsvtextoperationswidget.h>

qmdiEditor::qmdiEditor(QString fName, QWidget *p) : QsvTextEdit(p) {
    operationsWidget = new QsvTextOperationsWidget(this);

    setupActions();
    //	actionSave	= new QAction( QIcon(":images/save.png"), tr("&Save"), this );
    //	actionUndo	= new QAction( QIcon(":images/redo.png"), tr("&Redo"), this );
    //	actionRedo	= new QAction( QIcon(":images/undo.png"), tr("&Undo"), this );
    //	actionCopy	= new QAction( QIcon(":images/copy.png"), tr("&Copy"), this );
    //	actionCut	= new QAction( QIcon(":images/cut.png"), tr("&Cut"), this );
    //	actionPaste	= new QAction( QIcon(":images/paste.png"), tr("&Paste"), this  );

    actionSave = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    actionUndo = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
    actionRedo = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
    actionCopy = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
    actionCut = new QAction(QIcon::fromTheme("edit-cut"), tr("&Cut"), this);
    actionPaste = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
    actionFind = new QAction(QIcon::fromTheme("edit-find"), tr("&Find"), this);
    actionFindNext = new QAction(QIcon::fromTheme("go-next"), tr("Find &next"), this);
    actionFindPrev = new QAction(QIcon::fromTheme("go-previous"), tr("Find &previous"), this);
    actionReplace = new QAction(QIcon::fromTheme("edit-find-replace"), tr("&Replace"), this);
    actionGotoLine = new QAction(tr("&Goto line"), this);

    addAction(actionFind);
    addAction(actionFindNext);
    addAction(actionFindPrev);
    addAction(actionReplace);
    addAction(actionGotoLine);
    addAction(actionSave);
    addAction(actionUndo);
    addAction(actionCopy);
    addAction(actionCut);
    addAction(actionPaste);

    actionFind->setShortcut(QKeySequence::Find);
    actionFindNext->setShortcut(QKeySequence::FindNext);
    actionFindPrev->setShortcut(QKeySequence::FindPrevious);
    actionReplace->setShortcut(QKeySequence::Replace);
    actionGotoLine->setShortcut(QKeySequence("Ctrl+G"));

    actionSave->setObjectName("qmdiEditor::actionSave");
    actionUndo->setObjectName("qmdiEditor::actionUndo");
    actionRedo->setObjectName("qmdiEditor::actionRedo");
    actionCopy->setObjectName("qmdiEditor::actionCopy");
    actionCut->setObjectName("qmdiEditor::actionCut");
    actionPaste->setObjectName("qmdiEditor::actionPaste");
    actionFind->setObjectName("qmdiEditor::actionFind");
    actionFindNext->setObjectName("qmdiEditor::actionFindNext");
    actionFindPrev->setObjectName("qmdiEditor::actionFindPrev");
    actionReplace->setObjectName("qmdiEditor::actionReplace");
    actionGotoLine->setObjectName("qmdiEditor::actionGotoLine");

    connect(this, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));

    connect(actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), this, SLOT(redo()));
    connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(actionCut, SIGNAL(triggered()), this, SLOT(cut()));
    connect(actionPaste, SIGNAL(triggered()), this, SLOT(paste()));
    // 	connect( actiohAskHelp, SIGNAL(triggered()), this, SLOT(helpShowHelp()));
    connect(actionFind, SIGNAL(triggered()), operationsWidget, SLOT(showSearch()));
    connect(actionFindNext, SIGNAL(triggered()), operationsWidget, SLOT(searchNext()));
    connect(actionFindPrev, SIGNAL(triggered()), operationsWidget, SLOT(searchPrev()));
    connect(actionReplace, SIGNAL(triggered()), operationsWidget, SLOT(showReplace()));
    connect(actionGotoLine, SIGNAL(triggered()), operationsWidget, SLOT(showGotoLine()));

    textOperationsMenu = new QMenu(tr("Text actions"), this);
    textOperationsMenu->setObjectName("qmdiEditor::textOperationsMenu");
    textOperationsMenu->addAction(actionCapitalize);
    textOperationsMenu->addAction(actionLowerCase);
    textOperationsMenu->addAction(actionChangeCase);

    bookmarksMenu = new QMenu(tr("Bookmarks"), this);
    bookmarksMenu->setObjectName("qmdiEditor::bookmarksMenu ");
    bookmarksMenu->addAction(actionToggleBookmark);
    bookmarksMenu->addSeparator();
    bookmarksMenu->addAction(actionNextBookmark);
    bookmarksMenu->addAction(actionPrevBookmark);

    menus["&File"]->addAction(actionSave);

    menus["&Edit"]->addAction(actionUndo);
    menus["&Edit"]->addAction(actionRedo);
    menus["&Edit"]->addSeparator();
    menus["&Edit"]->addAction(actionCopy);
    menus["&Edit"]->addAction(actionCut);
    menus["&Edit"]->addAction(actionPaste);
    menus["&Edit"]->addSeparator();
    menus["&Edit"]->addMenu(textOperationsMenu);
    menus["&Edit"]->addMenu(bookmarksMenu);
    // 	menus["&Edit"]->addAction( actionTogglebreakpoint );
    menus["&Edit"]->addAction(actionFindMatchingBracket);

    menus["&Search"]->addAction(actionFind);
    menus["&Search"]->addAction(actionFindNext);
    menus["&Search"]->addAction(actionFindPrev);
    //	menus["&Search"]->addAction( actionClearSearchHighlight );
    menus["&Search"]->addSeparator();
    menus["&Search"]->addAction(actionReplace);
    menus["&Search"]->addSeparator();
    menus["&Search"]->addAction(actionGotoLine);

    this->toolbars[tr("main")]->addSeparator();
    this->toolbars[tr("main")]->addAction(actionSave);
    this->toolbars[tr("main")]->addAction(actionFind);
    this->toolbars[tr("main")]->addAction(actionReplace);
    this->toolbars[tr("main")]->addAction(actionFindPrev);
    this->toolbars[tr("main")]->addAction(actionFindNext);
    this->toolbars[tr("main")]->addAction(actionGotoLine);

    loadFile(fName);
    mdiClientName = getShortFileName();
}

qmdiEditor::~qmdiEditor() {
    // 	bookmarksMenu->deleteLater();
    // 	textOperationsMenu->deleteLater();
    // 	delete bookmarksMenu;
    // 	delete textOperationsMenu;
    mdiServer = nullptr;
}

QString qmdiEditor::getShortFileName() {
    if (getFileName().isEmpty()) {
        return "NO NAME";
    }

    // the name of the object for it's mdi server
    // is the file name alone, without the directory
    int i = getFileName().lastIndexOf('/');
    QString s;
    if (i != -1) {
        s = getFileName().mid(i + 1);
    } else {
        i = getFileName().lastIndexOf('\\');
        if (i != -1) {
            s = getFileName().mid(i + 1);
        } else {
            s = getFileName();
        }
    }
    return s;
}

bool qmdiEditor::canCloseClient() {
    if (!document()->isModified()) {
        return true;
    }

    // ask for saving
    QMessageBox msgBox(QMessageBox::Warning, tr("Application"),
                       tr("The document has been modified.\nDo you want to save your changes?"),
                       QMessageBox::Yes | QMessageBox::Default, this);

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        // 		TODO
        // 		return fileSave();
        return true;
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    // shut up GCC warnings
    return true;
}

/**
 * \brief return the mdi file opened
 * \return the value of LinesEditor::getFileName()
 *
 * This function is override from qmdiClient::mdiClientFileName(),
 * and will tell the qmdiServer which file is opened by this mdi client.
 */
QString qmdiEditor::mdiClientFileName() { return getFileName(); }
