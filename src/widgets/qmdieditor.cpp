/**
 * \file qmdieditor
 * \brief Implementation of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#include "qmdieditor.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QTextBlock>
#include <QTextDocument>
#include <qsvtextoperationswidget.h>

qmdiEditor::qmdiEditor(QString fName, QWidget *p) : Qutepart::Qutepart(p) {
    operationsWidget = new QsvTextOperationsWidget(this);

    QFont monospacedFont = this->font();
    monospacedFont.setPointSize(12);
    monospacedFont.setFamily("Monospace");
    setFont(monospacedFont);

    setLineWrapMode(LineWrapMode::NoWrap);

    setupActions();
    actionSave = new QAction(QIcon(":images/save.png"), tr("&Save"), this);
    actionUndo = new QAction(QIcon(":images/redo.png"), tr("&Redo"), this);
    actionRedo = new QAction(QIcon(":images/undo.png"), tr("&Undo"), this);
    actionCopy = new QAction(QIcon(":images/copy.png"), tr("&Copy"), this);
    actionCut = new QAction(QIcon(":images/cut.png"), tr("&Cut"), this);
    actionPaste = new QAction(QIcon(":images/paste.png"), tr("&Paste"), this);

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
    bookmarksMenu->addAction(toggleBookmarkAction());
    bookmarksMenu->addSeparator();
    bookmarksMenu->addAction(nextBookmarkAction());
    bookmarksMenu->addAction(prevBookmarkAction());

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
    // menus["&Edit"]->addAction(actionFindMatchingBracket);

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

void qmdiEditor::setupActions() {
    if (actionCapitalize) {
        delete actionCapitalize;
    }
    actionCapitalize = new QAction(tr("Change to &capital letters"), this);
    actionCapitalize->setObjectName("qsvEditor::actionCapitalize");
    actionCapitalize->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    connect(actionCapitalize, SIGNAL(triggered()), this, SLOT(transformBlockToUpper()));
    addAction(actionCapitalize);

    if (actionLowerCase) {
        delete actionLowerCase;
    }
    actionLowerCase = new QAction(tr("Change to &lower letters"), this);
    actionLowerCase->setObjectName("qsvEditor::actionLowerCase");
    actionLowerCase->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
    connect(actionLowerCase, SIGNAL(triggered()), this, SLOT(transformBlockToLower()));
    addAction(actionLowerCase);

    if (actionChangeCase) {
        delete actionChangeCase;
    }
    actionChangeCase = new QAction(tr("Change ca&se"), this);
    actionChangeCase->setObjectName("qsvEditor::actionChangeCase");
    connect(actionChangeCase, SIGNAL(triggered()), this, SLOT(transformBlockCase()));
    addAction(actionChangeCase);

    if (actionFindMatchingBracket) {
        delete actionFindMatchingBracket;
    }
    actionFindMatchingBracket = new QAction(tr("Find matching bracket"), this);
    actionFindMatchingBracket->setObjectName("qsvEditor::ctionFindMatchingBracket");
    actionFindMatchingBracket->setShortcuts(QList<QKeySequence>()
                                            << QKeySequence(Qt::CTRL | Qt::Key_6)
                                            << QKeySequence(Qt::CTRL | Qt::Key_BracketLeft)
                                            << QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    connect(actionFindMatchingBracket, SIGNAL(triggered()), this, SLOT(gotoMatchingBracket()));
    addAction(actionFindMatchingBracket);
}

void qmdiEditor::newDocument() { loadFile(""); }

int qmdiEditor::loadFile(const QString &fileName) {
    // clear older watches, and add a new one
    // QStringList sl = m_fileSystemWatcher->directories();
    // if (!sl.isEmpty()) {
    //     m_fileSystemWatcher->removePaths(sl);
    // }

    // bool modificationsEnabledState = getModificationsLookupEnabled();
    // setModificationsLookupEnabled(false);
    // hideBannerMessage();
    this->setReadOnly(false);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        QFileInfo fileInfo(file);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QApplication::restoreOverrideCursor();
            return -1;
        }

        QTextStream textStream(&file);
        setPlainText(textStream.readAll());
        file.close();

        auto langInfo = ::Qutepart::chooseLanguage(QString(), QString(), fileName);
        if (langInfo.isValid()) {
            setHighlighter(langInfo.id);
            setIndentAlgorithm(langInfo.indentAlg);
        }

        m_fileName = fileInfo.absoluteFilePath();
        // m_fileSystemWatcher->addPath(m_fileName);
        // if (!fileInfo.isWritable()) {
        //     this->setReadOnly(true);
        //     displayBannerMessage(
        //         tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and "
        //            "change the file attributes for write access'>here to force write
        //            access.</a>"));
        // }
    } else {
        m_fileName.clear();
        clear();
    }

    // setModificationsLookupEnabled(modificationsEnabledState);
    // removeModifications();

    QApplication::restoreOverrideCursor();
    return 0;
}

int qmdiEditor::saveFile(const QString &fileName) {
    // QStringList sl = m_fileSystemWatcher->directories();
    // if (!sl.isEmpty()) {
    //     m_fileSystemWatcher->removePaths(sl);
    // }
    // bool modificationsEnabledState = getModificationsLookupEnabled();
    // setModificationsLookupEnabled(false);
    // hideBannerMessage();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QTextStream textStream(&file);
    QTextBlock block = document()->begin();
    while (block.isValid()) {
        //		if (endOfLine==KeepOldStyle){
        textStream << block.text();
        // TODO WTF? - which type the file originally had?
        textStream << "\n";
        /*		} else {
                                QString s = block.text();
                                int i = s.length();

                                if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
                                        s = s.left( i-1 );
                                if (!s.isEmpty()) if ((s[i-1] == '\n') || (s[i-1] == '\r'))
                                        s = s.left( i-1 );
                                textStream << s;
                                switch (endOfLine) {
                                        case DOS:	textStream << "\r\n"; break;
                                        case Unix: 	textStream << "\n"; break;
                                        case Mac:	textStream << "\r"; break;
                                        default:	return 0; // just to keep gcc happy
                                }
                        }*/
        block = block.next();
    }
    file.close();

    m_fileName = fileName;
    // removeModifications();
    // setModificationsLookupEnabled(modificationsEnabledState);
    //	m_fileSystemWatcher->addPath(m_fileName);

    QApplication::restoreOverrideCursor();
    return 0;
}

// void QsvTextEdit::displayBannerMessage(QString message, int time) {
//     showUpperWidget(m_banner);
//     ui_banner->label->setText(message);
//     m_timerHideout = time;
//     QTimer::singleShot(1000, this, SLOT(on_hideTimer_timeout()));
// }

// void QsvTextEdit::hideBannerMessage() {
//     m_timerHideout = 0;
//     ui_banner->label->clear();
//     m_banner->hide();

//     // sometimes the top widget is displayed, lets workaround this
//     // TODO: find WTF this is happening
//     if (m_topWidget == m_banner) {
//         m_topWidget = nullptr;
//     }
// }

// int QsvTextEdit::saveFile() {
//     if (m_fileName.isEmpty()) {
//         return saveFileAs();
//     } else {
//         return saveFile(m_fileName);
//     }
// }

// int QsvTextEdit::saveFileAs() {
//     const QString lastDirectory;
//     QString s = QFileDialog::getSaveFileName(this, tr("Save file"), lastDirectory);
//     if (s.isEmpty()) {
//         return false;
//     }
//     return saveFile(s);
// }

void qmdiEditor::smartHome() {
    auto c = textCursor();
    int blockLen = c.block().text().length();
    if (blockLen == 0) {
        return;
    }

    int originalPosition = c.position();
    QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)
                                           ? QTextCursor::KeepAnchor
                                           : QTextCursor::MoveAnchor;
    c.movePosition(QTextCursor::StartOfLine, moveAnchor);
    int startOfLine = c.position();
    int i = 0;
    while (c.block().text()[i].isSpace()) {
        i++;
        if (i == blockLen) {
            i = 0;
            break;
        }
    }
    if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition)) {
        c.setPosition(startOfLine + i, moveAnchor);
    }
    setTextCursor(c);
}

void qmdiEditor::smartEnd() {
    QTextCursor c = textCursor();
    int blockLen = c.block().text().length();
    if (blockLen == 0) {
        return;
    }

    int originalPosition = c.position();
    QTextCursor::MoveMode moveAnchor = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)
                                           ? QTextCursor::KeepAnchor
                                           : QTextCursor::MoveAnchor;
    c.movePosition(QTextCursor::StartOfLine, moveAnchor);
    int startOfLine = c.position();
    c.movePosition(QTextCursor::EndOfLine, moveAnchor);
    int i = blockLen;
    while (c.block().text()[i - 1].isSpace()) {
        i--;
        if (i == 1) {
            i = blockLen;
            break;
        }
    }
    if ((originalPosition == startOfLine) || (startOfLine + i != originalPosition)) {
        c.setPosition(startOfLine + i, moveAnchor);
    }

    setTextCursor(c);
}

void qmdiEditor::transformBlockToUpper() {
    QTextCursor cursor = textCursor();
    QString s_before = cursor.selectedText();
    QString s_after = s_before.toUpper();

    if (s_before != s_after) {
        cursor.beginEditBlock();
        cursor.deleteChar();
        cursor.insertText(s_after);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void qmdiEditor::transformBlockToLower() {
    QTextCursor cursor = textCursor();
    QString s_before = cursor.selectedText();
    QString s_after = s_before.toLower();

    if (s_before != s_after) {
        cursor.beginEditBlock();
        cursor.deleteChar();
        cursor.insertText(s_after);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void qmdiEditor::transformBlockCase() {
    QTextCursor cursor = textCursor();
    QString s_before = cursor.selectedText();
    QString s_after = s_before;
    int s_len = s_before.length();

    for (int i = 0; i < s_len; i++) {
        QChar c = s_after[i];
        if (c.isLower()) {
            c = c.toUpper();
        } else if (c.isUpper()) {
            c = c.toLower();
        }
        s_after[i] = c;
    }

    if (s_before != s_after) {
        cursor.beginEditBlock();
        cursor.deleteChar();
        cursor.insertText(s_after);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void qmdiEditor::gotoMatchingBracket() {
#if 0
    /*
      WARNING: code duplication between this method and on_cursor_positionChanged();
      this needs to be refactored
     */

    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    int blockPosition = block.position();
    int cursorPosition = cursor.position();
    int relativePosition = cursorPosition - blockPosition;
    auto textBlock = block.text();

    if (relativePosition == textBlock.size()) {
        relativePosition--;
    }

    QChar currentChar = textBlock[relativePosition];

    // lets find it's partner
    // in theory, no errors should not happen, but one can never be too sure
    int j = m_config.matchBracketsList.indexOf(currentChar);
    if (j == -1) {
        return;
    }

    if (m_config.matchBracketsList[j] != m_config.matchBracketsList[j + 1]) {
        if (j % 2 == 0) {
            j = findMatchingChar(m_config.matchBracketsList[j], m_config.matchBracketsList[j + 1],
                                 true, block, cursorPosition);
        } else {
            j = findMatchingChar(m_config.matchBracketsList[j], m_config.matchBracketsList[j - 1],
                                 false, block, cursorPosition);
        }
    } else {
        j = findMatchingChar(m_config.matchBracketsList[j], m_config.matchBracketsList[j + 1], true,
                             block, cursorPosition);
    }

    cursor.setPosition(j);
    setTextCursor(cursor);
#endif
}

void qmdiEditor::gotoLine(int linenumber, int rownumber) {
    auto offset = document()->findBlockByLineNumber(linenumber).position();
    auto cursor = textCursor();
    cursor.setPosition(offset);
    cursor.movePosition(cursor.StartOfBlock);
    cursor.movePosition(cursor.Right, cursor.KeepAnchor, rownumber);
    setTextCursor(cursor);
    ensureCursorVisible();
}

bool qmdiEditor::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Resize) {
        emit widgetResized();
    }
    return Qutepart::eventFilter(obj, event);
}
