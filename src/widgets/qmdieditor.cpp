/**
 * \file qmdieditor
 * \brief Implementation of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QStyle>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextDocument>
#include <QToolBar>

#include "qmdieditor.h"
#include "qmdiserver.h"
#include "textoperationswidget.h"
#include "ui_bannermessage.h"

#if defined(WIN32)
#define DEFAULT_EDITOR_FONT "Courier new"
#else
#define DEFAULT_EDITOR_FONT "Monospace"
#endif

#include <QCompleter>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyledItemDelegate>

class BoldItemDelegate : public QStyledItemDelegate {
    // Q_OBJECT

  public:
    QString boldItemStr = "";
    BoldItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QString text = index.data(Qt::DisplayRole).toString();
        painter->save();

        bool isSelected = option.state & QStyle::State_Selected;
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.text().color());
        }

        QFont font = painter->font();
        if (text == boldItemStr) {
            font.setBold(true);
        }
        painter->setFont(font);
        QRect textRect = option.rect.adjusted(4, 0, -4, 0);
        // painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);

        painter->restore();
    }
};

namespace Qutepart {
QStringList getAvailableHighlihters() {
    extern QMap<QString, QString> languageNameToXmlFileName;
    return languageNameToXmlFileName.keys();
}
} // namespace Qutepart

qmdiEditor::qmdiEditor(QWidget *p) : QWidget(p) {
    textEditor = new Qutepart::Qutepart(this);
    operationsWidget = new QsvTextOperationsWidget(textEditor);
    mdiClientName = tr("NO NAME");
    fileSystemWatcher = new QFileSystemWatcher(this);
    QToolBar *toolbar = new QToolBar(this);
    QVBoxLayout *layout = new QVBoxLayout(this);

    setupActions();
    toolbar->addWidget(comboChangeHighlighter);
    toolbar->addWidget(buttonChangeIndenter);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);
    QLabel *staticLabel = new QLabel("", toolbar);
    toolbar->addWidget(staticLabel);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(textEditor);
    layout->addWidget(toolbar);

    connect(textEditor, &QPlainTextEdit::cursorPositionChanged, this, [this, staticLabel]() {
        QTextCursor cursor = textEditor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;
        staticLabel->setText(QString("%1:%2").arg(line).arg(column));
    });

    connect(fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(on_fileChanged(QString)));

    QFont monospacedFont = this->font();
    monospacedFont.setPointSize(10);
    monospacedFont.setFamily(DEFAULT_EDITOR_FONT);
    textEditor->setFont(monospacedFont);
    textEditor->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);

    banner = new QWidget(this);
    banner->setFont(QApplication::font());
    banner->hide();
    banner->setObjectName("banner");
    ui_banner = new Ui::BannerMessage;
    ui_banner->setupUi(banner);
    connect(ui_banner->label, SIGNAL(linkActivated(QString)), this,
            SLOT(fileMessage_clicked(QString)));

    textOperationsMenu = new QMenu(tr("Text actions"), this);
    textOperationsMenu->setObjectName("qmdiEditor::textOperationsMenu");
    textOperationsMenu->addAction(actionCapitalize);
    textOperationsMenu->addAction(actionLowerCase);
    textOperationsMenu->addAction(actionChangeCase);
    textOperationsMenu->addAction(textEditor->deleteLineAction());
    textOperationsMenu->addAction(textEditor->joinLinesAction());
    textOperationsMenu->addAction(textEditor->moveLineUpAction());
    textOperationsMenu->addAction(textEditor->moveLineDownAction());

    bookmarksMenu = new QMenu(tr("Bookmarks"), this);
    bookmarksMenu->setObjectName("qmdiEditor::bookmarksMenu");
    bookmarksMenu->addAction(textEditor->toggleBookmarkAction());
    bookmarksMenu->addSeparator();
    bookmarksMenu->addAction(textEditor->nextBookmarkAction());
    bookmarksMenu->addAction(textEditor->prevBookmarkAction());

    this->menus["&File"]->addAction(actionSave);
    this->menus["&File"]->addAction(actionSaveAs);
    this->menus["&Edit"]->addAction(actionUndo);
    this->menus["&Edit"]->addAction(actionRedo);
    this->menus["&Edit"]->addSeparator();
    this->menus["&Edit"]->addAction(actionCopy);
    this->menus["&Edit"]->addAction(actionCut);
    this->menus["&Edit"]->addAction(actionPaste);
    this->menus["&Edit"]->addSeparator();
    this->menus["&Edit"]->addMenu(textOperationsMenu);
    this->menus["&Edit"]->addMenu(bookmarksMenu);
    // menus["&Edit"]->addAction( actionTogglebreakpoint );
    // menus["&Edit"]->addAction(actionFindMatchingBracket);

    this->menus["&Search"]->addAction(actionFind);
    this->menus["&Search"]->addAction(actionFindNext);
    this->menus["&Search"]->addAction(actionFindPrev);
    // this->menus["&Search"]->addAction( actionClearSearchHighlight );
    this->menus["&Search"]->addSeparator();
    this->menus["&Search"]->addAction(actionReplace);
    this->menus["&Search"]->addSeparator();
    this->menus["&Search"]->addAction(actionGotoLine);

    this->toolbars[tr("main")]->addSeparator();
    this->toolbars[tr("main")]->addAction(actionSave);
    this->toolbars[tr("main")]->addAction(actionFind);
    this->toolbars[tr("main")]->addAction(actionReplace);
    this->toolbars[tr("main")]->addAction(actionGotoLine);
    this->toolbars[tr("main")]->addAction(actionFindPrev);
    this->toolbars[tr("main")]->addAction(actionFindNext);
}

qmdiEditor::~qmdiEditor() {
    bookmarksMenu->deleteLater();
    textOperationsMenu->deleteLater();
    mdiServer = nullptr;
}

QString qmdiEditor::getShortFileName() {
    if (getFileName().isEmpty()) {
        return tr("NO NAME");
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
    if (!textEditor->document()->isModified()) {
        return true;
    }

    QMessageBox msgBox(QMessageBox::Warning, mdiClientName,
                       tr("The document has been modified.\nDo you want to save your changes?"),
                       QMessageBox::Yes | QMessageBox::Default, this);

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        return doSave();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
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

/**
 * @brief Return status of editor
 * @return row, column,  zoom
 */
std::optional<std::tuple<int, int, int>> qmdiEditor::get_coordinates() const {
    auto cursor = textEditor->textCursor();
    auto row = textEditor->document()->findBlock(cursor.position()).blockNumber();
    auto col = cursor.columnNumber();
    auto zoom = font().pointSize();
    return std::make_tuple(row, col, zoom);
}

void qmdiEditor::setupActions() {
    comboChangeHighlighter = new QComboBox(this);
    buttonChangeIndenter = new QToolButton(this);
    actionSave = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    actionSaveAs = new QAction(QIcon::fromTheme("document-save-as"), tr("&Save as..."), this);
    actionUndo = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
    actionRedo = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
    actionCopy = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
    actionCut = new QAction(QIcon::fromTheme("edit-cut"), tr("&Cut"), this);
    actionPaste = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
    actionFind = new QAction(QIcon::fromTheme("edit-find"), tr("&Find"), this);
    actionFindNext = new QAction(QIcon::fromTheme("go-next"), tr("Find &next"), this);
    actionFindPrev = new QAction(QIcon::fromTheme("go-previous"), tr("Find &previous"), this);
    actionReplace = new QAction(QIcon::fromTheme("edit-find-replace"), tr("&Replace"), this);
    actionGotoLine = new QAction(QIcon::fromTheme("go-jump"), tr("&Goto line"), this);

    actionCapitalize = new QAction(tr("Change to &capital letters"), this);
    actionLowerCase = new QAction(tr("Change to &lower letters"), this);
    actionChangeCase = new QAction(tr("Change ca&se"), this);
    actionFindMatchingBracket = new QAction(tr("Find matching bracket"), this);

    actionSave->setShortcut(QKeySequence::Save);
    actionFind->setShortcut(QKeySequence::Find);
    actionFindNext->setShortcut(QKeySequence::FindNext);
    actionFindPrev->setShortcut(QKeySequence::FindPrevious);
    // actionReplace->setShortcut(QKeySequence::Replace);
    actionReplace->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    actionGotoLine->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    actionCapitalize->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    actionLowerCase->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
    actionFindMatchingBracket->setShortcuts(QList<QKeySequence>()
                                            << QKeySequence(Qt::CTRL | Qt::Key_6)
                                            << QKeySequence(Qt::CTRL | Qt::Key_BracketLeft)
                                            << QKeySequence(Qt::CTRL | Qt::Key_BracketRight));

    auto highlighters = Qutepart::getAvailableHighlihters();
    comboChangeHighlighter->setObjectName("qmdiEditor::comboChangeHighlighter");
    comboChangeHighlighter->addItems(highlighters);
    BoldItemDelegate *delegate = new BoldItemDelegate(comboChangeHighlighter);
    comboChangeHighlighter->setItemDelegate(delegate);

    buttonChangeIndenter->setObjectName("qmdiEditor::changeIndenter");
    buttonChangeIndenter->setText(tr("Change indentation"));
    buttonChangeIndenter->setPopupMode(QToolButton::InstantPopup);

    actionSave->setObjectName("qmdiEditor::actionSave");
    actionSaveAs->setObjectName("qmdiEditor::actionSaveAs");
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

    actionCapitalize->setObjectName("qsvEditor::actionCapitalize");
    actionLowerCase->setObjectName("qsvEditor::actionLowerCase");
    actionChangeCase->setObjectName("qsvEditor::actionChangeCase");
    actionFindMatchingBracket->setObjectName("qsvEditor::ctionFindMatchingBracket");

    connect(textEditor, &QPlainTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::redoAvailable, actionRedo, &QAction::setEnabled);

    QMenu *indentMenu = new QMenu(tr("Indentation"), this);
    QActionGroup *indentGroup = new QActionGroup(this);
    indentGroup->addAction(indentMenu->addAction(tr("None")))->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("Normal - using previous line")))
        ->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("C-Style indentation")))->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("Lisp based indentatio")))->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("Scheme based indentation")))
        ->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("XML based indentation")))->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("Python based indentation")))
        ->setCheckable(true);
    indentGroup->addAction(indentMenu->addAction(tr("Ruby based indentation")))->setCheckable(true);
    indentGroup->setExclusive(true);

    buttonChangeIndenter->setMenu(indentMenu);
    connect(indentGroup, &QActionGroup::triggered, this, &qmdiEditor::chooseIndenter);
    connect(indentMenu, &QMenu::aboutToShow, this, &qmdiEditor::updateIndenterMenu);

    connect(comboChangeHighlighter, &QComboBox::currentTextChanged, this,
            &qmdiEditor::chooseHighliter);
    connect(actionSave, &QAction::triggered, this, &qmdiEditor::doSave);
    connect(actionSaveAs, &QAction::triggered, this, &qmdiEditor::doSaveAs);
    connect(actionUndo, &QAction::triggered, textEditor, &QPlainTextEdit::undo);
    connect(actionRedo, &QAction::triggered, textEditor, &QPlainTextEdit::redo);
    connect(actionCopy, &QAction::triggered, textEditor, &QPlainTextEdit::copy);
    connect(actionCut, &QAction::triggered, textEditor, &QPlainTextEdit::cut);
    connect(actionPaste, &QAction::triggered, textEditor, &QPlainTextEdit::paste);
    connect(actionFind, &QAction::triggered, operationsWidget,
            &QsvTextOperationsWidget::showSearch);
    connect(actionFindNext, &QAction::triggered, operationsWidget,
            &QsvTextOperationsWidget::searchNext);
    connect(actionFindPrev, &QAction::triggered, operationsWidget,
            &QsvTextOperationsWidget::searchPrev);
    connect(actionReplace, &QAction::triggered, operationsWidget,
            &QsvTextOperationsWidget::showReplace);
    connect(actionGotoLine, &QAction::triggered, operationsWidget,
            &QsvTextOperationsWidget::showGotoLine);

    connect(actionCapitalize, &QAction::triggered, this, &qmdiEditor::transformBlockToUpper);
    connect(actionLowerCase, &QAction::triggered, this, &qmdiEditor::transformBlockToLower);
    connect(actionChangeCase, &QAction::triggered, this, &qmdiEditor::transformBlockCase);
    connect(actionFindMatchingBracket, &QAction::triggered, this, &qmdiEditor::gotoMatchingBracket);
    // 	connect( actiohAskHelp, SIGNAL(triggered()), this, SLOT(helpShowHelp()));

    addAction(actionSave);
    addAction(actionSaveAs);
    addAction(actionUndo);
    addAction(actionRedo);
    addAction(actionCopy);
    addAction(actionCut);
    addAction(actionPaste);
    addAction(actionFind);
    addAction(actionFindNext);
    addAction(actionFindPrev);
    addAction(actionReplace);
    addAction(actionGotoLine);

    addAction(actionCapitalize);
    addAction(actionLowerCase);
    addAction(actionChangeCase);
    addAction(actionFindMatchingBracket);

    // default is control+b - which we want to use for build
    textEditor->toggleBookmarkAction()->setShortcut(QKeySequence());
}

bool qmdiEditor::getModificationsLookupEnabled() { return fileModifications; }

void qmdiEditor::setModificationsLookupEnabled(bool value) { fileModifications = value; }

void qmdiEditor::on_fileChanged(const QString &filename) {
    if (this->fileName != filename) {
        return;
    }

    if (!fileModifications) {
        return;
    }

    QFileInfo f(filename);
    QString message;
    if (f.exists()) {
        message = QString("%1 <a href=':reload' title='%2'>%3</a>")
                      .arg(tr("File has been modified outside the editor"),
                           tr("Clicking this links will revert all changes to this editor"),
                           tr("Click here to reload"));
    } else {
        message = tr("File has been deleted outside the editor.");
    }
    displayBannerMessage(message, 10);
}

void qmdiEditor::adjustBottomAndTopWidget() {
    if (topWidget) {
        QWidget *parent = textEditor->viewport();
        QRect r = parent->rect();
        topWidget->adjustSize();
        r.adjust(10, 0, -10, 0);
        r.setHeight(topWidget->height());
        r.moveTop(10);
        r.moveLeft(parent->pos().x() + 10);
        topWidget->setGeometry(r);
        topWidget->show();
    }
    if (bottomWidget) {
        QWidget *parent = textEditor->viewport();
        QRect r = parent->rect();
        bottomWidget->adjustSize();
        r.adjust(10, 0, -10, 0);
        r.setHeight(bottomWidget->height());
        r.moveBottom(parent->rect().height() - 10);
        r.moveLeft(parent->pos().x() + 10);
        bottomWidget->setGeometry(r);
        bottomWidget->show();
    }
}

void qmdiEditor::hideTimer_timeout() {
    if (m_timerHideout != 0) {
        QString s;
        s.setNum(m_timerHideout);
        m_timerHideout--;
        ui_banner->timer->setText(s);
        QTimer::singleShot(1000, this, SLOT(hideTimer_timeout()));
    } else {
        ui_banner->timer->clear();
        banner->hide();
    }
}

void qmdiEditor::showUpperWidget(QWidget *w) {
    topWidget = w;
    adjustBottomAndTopWidget();
}

void qmdiEditor::showBottomWidget(QWidget *w) {
    bottomWidget = w;
    adjustBottomAndTopWidget();
}

void qmdiEditor::displayBannerMessage(QString message, int time) {
    showUpperWidget(banner);
    ui_banner->label->setText(message);
    m_timerHideout = time;
    QTimer::singleShot(1000, this, SLOT(hideTimer_timeout()));
}

void qmdiEditor::hideBannerMessage() {
    m_timerHideout = 0;
    ui_banner->label->clear();
    banner->hide();

    // sometimes the top widget is displayed, lets workaround this
    // TODO: find WTF this is happening
    if (topWidget == banner) {
        topWidget = nullptr;
    }
}

void qmdiEditor::newDocument() { loadFile(""); }

bool qmdiEditor::doSave() {
    if (fileName.isEmpty()) {
        return doSaveAs();
    } else {
        return saveFile(fileName);
    }
}

bool qmdiEditor::doSaveAs() {
    const QString lastDirectory;
    QString s = QFileDialog::getSaveFileName(this, tr("Save file"), lastDirectory);
    if (s.isEmpty()) {
        return false;
    }
    return saveFile(s);
}

bool qmdiEditor::loadFile(const QString &newFileName) {
    // clear older watches, and add a new one
    QStringList sl = fileSystemWatcher->directories();
    if (!sl.isEmpty()) {
        fileSystemWatcher->removePaths(sl);
    }

    bool modificationsEnabledState = getModificationsLookupEnabled();
    setModificationsLookupEnabled(false);
    hideBannerMessage();
    textEditor->setReadOnly(false);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
    if (!newFileName.isEmpty()) {
        QFile file(newFileName);
        QFileInfo fileInfo(file);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QApplication::restoreOverrideCursor();
            return false;
        }

        QTextStream textStream(&file);
        textEditor->setPlainText(textStream.readAll());
        file.close();

        this->fileName = fileInfo.absoluteFilePath();
        fileSystemWatcher->addPath(newFileName);
        if (!fileInfo.isWritable()) {
            textEditor->setReadOnly(true);
            displayBannerMessage(
                tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and "
                   "change the file attributes for write access'>here to force write access.</a>"),
                10);
        }
    } else {
        this->fileName.clear();
        textEditor->clear();
    }

    mdiClientName = getShortFileName();
    fileName = newFileName;

    updateFileDetails();
    setModificationsLookupEnabled(modificationsEnabledState);
    // removeModifications();

    QApplication::restoreOverrideCursor();
    return true;
}

QString cleanUpLine(QString str, bool cleanupTrailingSpaces) {
    while (!str.isEmpty() && (str.endsWith('\n') || str.endsWith('\r'))) {
        str.chop(1);
    }

    if (cleanupTrailingSpaces) {
        str = str.trimmed();
    }

    return str;
}

bool qmdiEditor::saveFile(const QString &newFileName) {
    QStringList sl = fileSystemWatcher->directories();
    if (!sl.isEmpty()) {
        fileSystemWatcher->removePaths(sl);
    }

    bool modificationsEnabledState = getModificationsLookupEnabled();
    setModificationsLookupEnabled(false);
    hideBannerMessage();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFile file(newFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    auto textStream = QTextStream(&file);
    auto block = textEditor->document()->begin();
    auto stopProcessing = false;
    QTextCursor cursor(textEditor->document());
    while (block.isValid()) {
        QString s = block.text();
        s = cleanUpLine(s, trimSpacesOnSave);
        if (trimSpacesOnSave) {
            cursor.setPosition(block.position());
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.insertText(s);
        }

        switch (endLineStyle) {
        case EndLineStyle::WindowsEndLine:
            textStream << s;
            textStream << "\r\n";
            break;
        case EndLineStyle::UnixEndLine:
            textStream << s;
            textStream << "\n";
            break;
        case KeepOriginalEndline:
            // TODO - this is broken
            textStream << textEditor->document()->toPlainText();
            stopProcessing = true;
            break;
        }

        if (stopProcessing) {
            break;
        }
        block = block.next();
    }
    file.close();
    textEditor->document()->setModified(false);

    QApplication::processEvents();
    QApplication::restoreOverrideCursor();

    this->fileName = newFileName;
    this->mdiClientName = getShortFileName();
    // removeModifications();
    fileSystemWatcher->addPath(newFileName);
    setModificationsLookupEnabled(modificationsEnabledState);

    auto w = dynamic_cast<QTabWidget *>(this->mdiServer);
    auto i = w->indexOf(this);
    w->setTabText(i, mdiClientName);
    w->setTabToolTip(i, mdiClientFileName());
    updateFileDetails();
    return true;
}

void qmdiEditor::smartHome() {
    auto c = textEditor->textCursor();
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
    textEditor->setTextCursor(c);
}

void qmdiEditor::smartEnd() {
    QTextCursor c = textEditor->textCursor();
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

    textEditor->setTextCursor(c);
}

void qmdiEditor::transformBlockToUpper() {
    QTextCursor cursor = textEditor->textCursor();
    QString s_before = cursor.selectedText();
    QString s_after = s_before.toUpper();

    if (s_before != s_after) {
        cursor.beginEditBlock();
        cursor.deleteChar();
        cursor.insertText(s_after);
        cursor.endEditBlock();
        textEditor->setTextCursor(cursor);
    }
}

void qmdiEditor::transformBlockToLower() {
    QTextCursor cursor = textEditor->textCursor();
    QString s_before = cursor.selectedText();
    QString s_after = s_before.toLower();

    if (s_before != s_after) {
        cursor.beginEditBlock();
        cursor.deleteChar();
        cursor.insertText(s_after);
        cursor.endEditBlock();
        textEditor->setTextCursor(cursor);
    }
}

void qmdiEditor::transformBlockCase() {
    QTextCursor cursor = textEditor->textCursor();
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
        textEditor->setTextCursor(cursor);
    }
}

void qmdiEditor::fileMessage_clicked(const QString &s) {
    if (s == ":reload") {
        loadFile(fileName);
        hideBannerMessage();
    } else if (s == ":forcerw") {
        hideBannerMessage();
        textEditor->setReadOnly(false);
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

void qmdiEditor::chooseHighliter(const QString &newText) {
    auto langInfo = ::Qutepart::chooseLanguage(QString(), newText, {});
    if (langInfo.isValid()) {
        textEditor->setHighlighter(langInfo.id);
        // textEditor->setIndentAlgorithm(langInfo.indentAlg);
        // buttonChangeIndenter->menu()->actions().at(langInfo.indentAlg)->setChecked(true);
    }
}

void qmdiEditor::chooseIndenter(QAction *action) {
    buttonChangeIndenter->setText(action->text());
    auto act = buttonChangeIndenter->menu()->actions();
    auto j = act.indexOf(action);
    textEditor->setIndentAlgorithm(static_cast<Qutepart::IndentAlg>(j));
}

void qmdiEditor::updateFileDetails() {
    auto langInfo = ::Qutepart::chooseLanguage(QString(), QString(), fileName);
    if (langInfo.isValid()) {
        textEditor->setHighlighter(langInfo.id);
        textEditor->setIndentAlgorithm(langInfo.indentAlg);
        buttonChangeIndenter->menu()->actions().at(langInfo.indentAlg)->setChecked(true);

        auto delegate = static_cast<BoldItemDelegate *>(comboChangeHighlighter->itemDelegate());
        delegate->boldItemStr = langInfo.names[0];

        auto i = comboChangeHighlighter->findText(delegate->boldItemStr);
        if (i > 0) {
            comboChangeHighlighter->setCurrentIndex(i);
        }

        auto s = buttonChangeIndenter->menu()->actions()[langInfo.indentAlg]->text();
        buttonChangeIndenter->setText(s);
    }
    // TODO else ...? what should we do if the file is not recognized?
}

void qmdiEditor::updateIndenterMenu() {
    auto langInfo = ::Qutepart::chooseLanguage(QString(), QString(), this->getFileName());
    auto k = 0;
    for (auto i : buttonChangeIndenter->menu()->actions()) {
        QFont font = i->font();
        if (langInfo.isValid() && k == langInfo.indentAlg) {
            font.setBold(true);
        } else {
            font.setBold(false);
        }
        i->setFont(font);
        k++;
    }
}

void qmdiEditor::updateHighlighterMenu() {
    // todo
}
