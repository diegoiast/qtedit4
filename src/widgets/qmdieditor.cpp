/**
 * \file qmdieditor.cpp
 * \brief Implementation of the qmdiEditor
 * \author Diego Iastrubni diegoiast@gmail.com
 * \see Qutepart qmdiClient
 */

// SPDX-License-Identifier: MIT

#include <QActionGroup>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QFutureWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPromise>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolBar>
#include <QToolTip>
#include <QTreeView>
#include <QtConcurrent/QtConcurrent>

#include <pluginmanager.h>
#include <qmdiserver.h>
#include <qmditabwidget.h>

#include "GlobalCommands.hpp"
#include "plugins/texteditor/thememanager.h"
#include "qmdieditor.h"
#include "widgets/textoperationswidget.h"
#include "widgets/textpreview.h"
#include "widgets/ui_bannermessage.h"

#define PLAIN_TEXT_HIGHIGHTER "Plain text"

#if defined(WIN32)
#define PLATFORM_LINE_ENDING "\r\n"
#else
#define PLATFORM_LINE_ENDING "\n"
#endif

auto static is_running_under_gnome() -> bool {
#if defined(WIN32)
    return false;
#else
    const auto desktop_session = std::getenv("DESKTOP_SESSION");
    const auto xdg_current_desktop = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktop_session && std::string(desktop_session).find("gnome") != std::string::npos) {
        return true;
    }
    if (xdg_current_desktop &&
        std::string(xdg_current_desktop).find("GNOME") != std::string::npos) {
        return true;
    }
    return false;
#endif
}

auto static getCorrespondingFile(const QString &fileName) -> QString {
    auto static const cExtensions = QStringList{"c", "cpp", "cxx", "cc", "c++"};
    auto static const headerExtensions = QStringList{"h", "hpp", "hh"};

    auto fileInfo = QFileInfo(fileName);
    auto baseName = fileInfo.baseName();
    auto dirPath = fileInfo.absolutePath();

    if (cExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
        for (const auto &headerExt : headerExtensions) {
            auto headerFileName = dirPath + QDir::separator() + baseName + "." + headerExt;
            if (QFileInfo::exists(headerFileName)) {
                return headerFileName;
            }
        }
    } else if (headerExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
        for (const auto &cExt : cExtensions) {
            auto cFileName = dirPath + QDir::separator() + baseName + "." + cExt;
            if (QFileInfo::exists(cFileName)) {
                return cFileName;
            }
        }
    }
    return {};
}

auto static getLineEnding(QIODevice &stream, const QString &defaultLineEnding) -> QString {
    if (stream.atEnd()) {
        return defaultLineEnding;
    }

    auto ending = QString{};
    auto pos = stream.pos();
    while (!stream.atEnd()) {
        QChar ch = stream.read(1).at(0);
        if (ch == '\r' || ch == '\n') {
            ending += ch;
            if (!stream.atEnd()) {
                ch = stream.read(1).at(0);
                if (ch == '\r' || ch == '\n') {
                    if (ch != ending[0]) {
                        ending += ch;
                    }
                }
            }
            break;
        }
    }
    stream.seek(pos);
    return ending;
}

static auto createSubFollowSymbolSubmenu(const CommandArgs &data, QMenu *menu,
                                         PluginManager *manager) -> void {
    static auto const START_MARKER = QString("/^");
    static auto const END_MARKER = QString("$/;\"");
    static auto const MIN_LENGTH = START_MARKER.length() + END_MARKER.length();

    if (!data.contains(GlobalArguments::Tags)) {
        return;
    }

    auto tags = data[GlobalArguments::Tags].toList();
    auto originalSymbol = data[GlobalArguments::Symbol].toString();
    {
        auto a = new QAction(originalSymbol, menu);
        a->setEnabled(false);

        if (tags.isEmpty()) {
            a->setText(QObject::tr("%1 - not found").arg(originalSymbol));
            menu->addAction(a);
            return;
        }
        menu->addAction(a);
    }

    for (const QVariant &item : std::as_const(tags)) {
        auto const tag = item.toHash();
        auto const fileName = tag[GlobalArguments::FileName].toString();
        auto const fieldType = tag[GlobalArguments::Type].toString();
        auto const fieldValue = tag[GlobalArguments::Value].toString();
        auto const rawAddress = tag[GlobalArguments::Raw].toString();
        auto address = rawAddress;
        if (address.startsWith(START_MARKER) && address.endsWith(END_MARKER) &&
            address.length() > MIN_LENGTH) {
            address = address.mid(START_MARKER.length(), address.length() - MIN_LENGTH);
        }

        auto fi = QFileInfo(fileName);
        auto simpleFileName = fi.fileName();
        auto title = QString("%1 - %2 %3").arg(simpleFileName, fieldType, fieldValue);
        auto a = new QAction(title, menu);
        QObject::connect(a, &QAction::triggered, a, [fileName, rawAddress, address, manager]() {
            auto nativeFileName = QDir::toNativeSeparators(fileName);
            manager->openFile(nativeFileName);
            auto client = manager->clientForFileName(nativeFileName);
            auto editor = dynamic_cast<qmdiEditor *>(client);
            if (editor) {
                editor->loadContent();
                if (!address.isEmpty()) {
                    editor->findText(address);
                }
                editor->setFocus();
            }
        });
        menu->addAction(a);
    }
}

class BoldItemDelegate : public QStyledItemDelegate {
  public:
    QString boldItemStr = "";
    explicit BoldItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

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
    auto l = QList<QString>();
    l.append(PLAIN_TEXT_HIGHIGHTER);
    l.append(languageNameToXmlFileName.keys());
    return l;
}
} // namespace Qutepart

qmdiEditor::qmdiEditor(QWidget *p, Qutepart::ThemeManager *themes)
    : QWidget(p), themeManager(themes) {
    textEditor = new Qutepart::Qutepart(this);
    operationsWidget = new TextOperationsWidget(this, textEditor);
    mdiClientName = tr("NO NAME");
    fileSystemWatcher = new QFileSystemWatcher(this);
    toolbar = new QWidget(this);
    auto layout2 = new QHBoxLayout(toolbar);
    auto layout = new QVBoxLayout(this);

    // Set up completion callback
    textEditor->setCompletionCallback([this](const QString &prefix) {
        if (prefix.length() < 2) {
            return QSet<QString>();
        }
        auto c = this->getTagCompletions(prefix);
        return c;
    });

    operationsWidget->hide();
    setupActions();
    layout2->setSpacing(0);
    layout2->setContentsMargins(0, 2, 0, 2);
    toolbar->setLayout(layout2);
    toolbar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    toolbar->layout()->addWidget(comboChangeHighlighter);
    toolbar->layout()->addWidget(buttonChangeIndenter);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    QWidget *spacer = new QWidget(this);
    QLabel *staticLabel = new QLabel("", this);

    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->layout()->addWidget(spacer);
    toolbar->layout()->addWidget(staticLabel);
    toolbar->layout()->addWidget(previewButton);

    textPreview = new TextPreview(this);
    textPreview->setVisible(false);

    textEditor->setFrameStyle(QFrame::NoFrame);
    splitter->addWidget(textEditor);
    splitter->addWidget(textPreview);

    connect(textEditor, &QPlainTextEdit::cursorPositionChanged, this, [this, staticLabel]() {
        QTextCursor cursor = textEditor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;
        staticLabel->setText(QString("%1:%2").arg(line).arg(column));
    });

    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &qmdiEditor::on_fileChanged);
    fileModifications = true;

    auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setFixedPitch(true);
    fnt.setPointSize(14);
    setEditorFont(fnt);

    textEditor->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);

    banner = new QWidget(this);
    banner->hide();
    banner->setObjectName("banner");
    ui_banner = new Ui::BannerMessage;
    ui_banner->setupUi(banner);
    connect(ui_banner->label, &QLabel::linkActivated, this, &qmdiEditor::fileMessage_clicked);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(toolbar);
    layout->addWidget(banner);
    layout->addWidget(splitter);
    layout->addWidget(operationsWidget);

    textOperationsMenu = new QMenu(tr("Text actions"), this);
    textOperationsMenu->setObjectName("qmdiEditor::textOperationsMenu");
    textOperationsMenu->addAction(textEditor->deleteLineAction());
    textOperationsMenu->addAction(textEditor->cutLineAction());
    textOperationsMenu->addAction(textEditor->copyLineAction());
    textOperationsMenu->addAction(textEditor->pasteLineAction());
    textOperationsMenu->addSeparator();
    textOperationsMenu->addAction(actionCapitalize);
    textOperationsMenu->addAction(actionLowerCase);
    textOperationsMenu->addAction(actionChangeCase);
    textOperationsMenu->addAction(textEditor->joinLinesAction());
    textOperationsMenu->addAction(textEditor->duplicateSelectionAction());
    textOperationsMenu->addAction(textEditor->deleteLineAction());
    textOperationsMenu->addAction(textEditor->moveLineUpAction());
    textOperationsMenu->addAction(textEditor->moveLineDownAction());
    textOperationsMenu->addAction(textEditor->toggleCommentAction());
    textOperationsMenu->addSeparator();
    textOperationsMenu->addAction(actionToggleHeader);

    bookmarksMenu = new QMenu(tr("Bookmarks"), this);
    bookmarksMenu->setObjectName("qmdiEditor::bookmarksMenu");
    bookmarksMenu->addAction(textEditor->toggleBookmarkAction());
    bookmarksMenu->addSeparator();
    bookmarksMenu->addAction(textEditor->nextBookmarkAction());
    bookmarksMenu->addAction(textEditor->prevBookmarkAction());

    foldingMenu = new QMenu(tr("Folding"), this);
    foldingMenu->setObjectName("foldingMenu");
    foldingMenu->addAction(textEditor->foldAction());
    foldingMenu->addAction(textEditor->unfoldAction());
    foldingMenu->addAction(textEditor->foldTopLevelAction());
    foldingMenu->addAction(textEditor->unfoldAllAction());
    foldingMenu->addAction(textEditor->toggleFoldAction());

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
    this->menus["&Edit"]->addMenu(foldingMenu);
    menus["&Edit"]->addAction(textEditor->findMatchingBracketAction());

    this->menus["&Search"]->addAction(actionFind);
    this->menus["&Search"]->addAction(actionFindNext);
    this->menus["&Search"]->addAction(actionFindPrev);
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

    this->contextMenu.addSeparator();
    this->contextMenu.addAction(actionCopyFileName);
    this->contextMenu.addAction(actionCopyFilePath);

    this->installEventFilter(this);
    this->textEditor->setMouseTracking(true);
    this->textEditor->viewport()->installEventFilter(this);

    this->originalLineEnding = PLATFORM_LINE_ENDING;

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setSingleShot(true);
    autoSaveTimer->setInterval(5000);
    connect(autoSaveTimer, &QTimer::timeout, this, &qmdiEditor::autoSave);
    connect(textEditor, &QPlainTextEdit::textChanged, this, [this]() { autoSaveTimer->start(); });

    QByteArray randomData;
    for (auto i = 0; i < 16; ++i) {
        randomData.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    auto hash = QCryptographicHash::hash(randomData, QCryptographicHash::Sha1).toHex();
    uid = QString::fromLatin1(hash);
}

qmdiEditor::~qmdiEditor() {
    bookmarksMenu->deleteLater();
    textOperationsMenu->deleteLater();
    mdiServer = nullptr;
}

QString qmdiEditor::getShortFileName() {
    if (fileName.isEmpty()) {
        return tr("NO NAME");
    }

    // the name of the object for it's mdi server
    // is the file name alone, without the directory
    int i = fileName.lastIndexOf('/');
    QString s;
    if (i != -1) {
        s = fileName.mid(i + 1);
    } else {
        i = fileName.lastIndexOf('\\');
        if (i != -1) {
            s = fileName.mid(i + 1);
        } else {
            s = fileName;
        }
    }
    return s;
}

void qmdiEditor::showContextMenu(const QPoint &localPosition, const QPoint &globalPosition) {
    auto followSymbolMenu = new QMenu(this);
    followSymbolMenu->setTitle(tr("Follow symbol"));

    auto loadingAction = new QAction(tr("Loading..."), followSymbolMenu);
    loadingAction->setEnabled(false);
    followSymbolMenu->addAction(loadingAction);

    auto menu = textEditor->createStandardContextMenu();
    auto separator = new QAction(this);
    auto actions = menu->actions();
    auto firstAction = actions.isEmpty() ? nullptr : actions.first();

    separator->setSeparator(true);
    menu->insertAction(firstAction, separator);
    menu->insertMenu(firstAction, followSymbolMenu);

    auto future = getSuggestionsForCurrentWord(localPosition);
    auto watcher = new QFutureWatcher<CommandArgs>(menu);
    connect(watcher, &QFutureWatcher<CommandArgs>::finished, this, [=, this]() {
        followSymbolMenu->removeAction(loadingAction);
        delete loadingAction;
        auto res = watcher->result();
        auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
        createSubFollowSymbolSubmenu(res, followSymbolMenu, pluginManager);
    });

    watcher->setFuture(future);
    menu->exec(globalPosition);
    delete menu;
}

bool qmdiEditor::canCloseClient() {
    if (!textEditor->document()->isModified()) {
        deleteBackup();
    } else {
        saveBackup();
    }
    return true;

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
    deleteBackup();
    return true;
}

QString qmdiEditor::mdiClientFileName() { return fileName; }

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

qmdiClientState qmdiEditor::getState() const {
    auto cursor = textEditor->textCursor();
    auto row = textEditor->document()->findBlock(cursor.position()).blockNumber();
    auto col = cursor.columnNumber();
    auto zoom = font().pointSize();
    auto state = qmdiClientState();
    state[StateConstants::COLUMN] = col;
    state[StateConstants::ROW] = row;
    state[StateConstants::ZOOM] = zoom;

    if (!uid.isEmpty()) {
        state[StateConstants::UUID] = uid;
    }

    if (cursor.hasSelection()) {
        state[StateConstants::SEL_ANCHOR] = cursor.anchor();
        state[StateConstants::SEL_POSITION] = cursor.position();
    } else {
        state.remove(StateConstants::SEL_ANCHOR);
        state.remove(StateConstants::SEL_POSITION);
    }
    return state;
}

void qmdiEditor::setState(const qmdiClientState &state) {
    savedState = state;
    if (!documentHasBeenLoaded) {
        return;
    }

    if (state.contains(StateConstants::UUID)) {
        uid = state[StateConstants::UUID].toString();
    }

    if (state.contains(StateConstants::ZOOM)) {
        auto zoom = state[StateConstants::ZOOM].toInt();
        auto f = font();
        f.setPointSize(zoom);
        setFont(f);
    }

    {
        auto col = 0;
        auto row = 0;
        if (state.contains(StateConstants::COLUMN)) {
            col = state[StateConstants::COLUMN].toInt();
        }
        if (state.contains(StateConstants::ROW)) {
            row = state[StateConstants::ROW].toInt();
        }
        textEditor->goTo(col, row);
    }

    if (state.contains(StateConstants::SEL_ANCHOR) &&
        state.contains(StateConstants::SEL_POSITION)) {
        auto anchor = state[StateConstants::SEL_ANCHOR].toInt();
        auto position = state[StateConstants::SEL_POSITION].toInt();
        auto cursor = textEditor->textCursor();
        cursor.setPosition(anchor, QTextCursor::MoveAnchor);
        cursor.setPosition(position, QTextCursor::KeepAnchor);
        textEditor->setTextCursor(cursor);
    }
}

void qmdiEditor::setupActions() {
    buttonChangeIndenter = new QToolButton(this);
    comboChangeHighlighter = new QComboBox(this);
    previewButton = new QPushButton(this);
    previewButton->setText(tr("Preview"));
    previewButton->setFlat(true);
    previewButton->setCheckable(true);

    connect(previewButton, &QAbstractButton::toggled, this, [this](bool toggled) {
        this->textPreview->setVisible(toggled);
        if (toggled) {
            updatePreview();
        }
    });

    actionSave = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    actionSaveAs = new QAction(QIcon::fromTheme("document-save-as"), tr("Save &as..."), this);
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
    actionCopyFileName = new QAction(tr("Copy filename to clipboard"), this);
    actionCopyFilePath = new QAction(tr("Copy full path to clipboard"), this);

    actionCapitalize = new QAction(tr("Change to &capital letters"), this);
    actionLowerCase = new QAction(tr("Change to &lower letters"), this);
    actionChangeCase = new QAction(tr("Change ca&se"), this);
    actionToggleHeader = new QAction(tr("Toggle header/implementation"), this);

    actionSave->setShortcut(QKeySequence::Save);
    actionSaveAs->setShortcut(QKeySequence::SaveAs);
    actionUndo->setShortcut(QKeySequence::Undo);
    actionRedo->setShortcut(QKeySequence::Redo);
    actionCopy->setShortcut(QKeySequence::Copy);
    actionCut->setShortcut(QKeySequence::Cut);
    actionPaste->setShortcut(QKeySequence::Paste);
    actionFind->setShortcut(QKeySequence::Find);
    actionFindNext->setShortcut(QKeySequence::FindNext);
    actionFindPrev->setShortcut(QKeySequence::FindPrevious);
    // this is usually "control+r, which we use for running a target
    // actionReplace->setShortcut(QKeySequence::Replace);
    actionReplace->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    if (!is_running_under_gnome()) {
        actionGotoLine->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    } else {
        actionGotoLine->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    }
    actionCapitalize->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    actionLowerCase->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
    actionToggleHeader->setShortcut(Qt::Key_F4);

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
    actionCapitalize->setObjectName("qmdiEditor::actionCapitalize");
    actionLowerCase->setObjectName("qmdiEditor::actionLowerCase");
    actionChangeCase->setObjectName("qmdiEditor::actionChangeCase");
    actionToggleHeader->setObjectName("qmdiEditor::actiohToggleHeader");

    actionSave->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionSaveAs->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionUndo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionRedo->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCopy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCut->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionPaste->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionFind->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionFindNext->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionFindPrev->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionReplace->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionGotoLine->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionCapitalize->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionLowerCase->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionChangeCase->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    actionToggleHeader->setShortcutContext(Qt::WidgetWithChildrenShortcut);

    connect(textEditor, &QPlainTextEdit::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::redoAvailable, actionRedo, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::textChanged, this, &qmdiEditor::updateClientName);

    connect(actionCopyFileName, &QAction::triggered, actionCopyFileName, [this]() {
        auto c = QApplication::clipboard();
        c->setText(getShortFileName());
    });
    connect(actionCopyFilePath, &QAction::triggered, actionCopyFilePath, [this]() {
        auto c = QApplication::clipboard();
        c->setText(fileName);
    });

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
    connect(actionCopy, &QAction::triggered, textEditor, &Qutepart::Qutepart::multipleCursorCopy);
    connect(actionCut, &QAction::triggered, textEditor, &Qutepart::Qutepart::multipleCursorCut);
    connect(actionPaste, &QAction::triggered, textEditor, &Qutepart::Qutepart::multipleCursorPaste);
    connect(actionFind, &QAction::triggered, operationsWidget, &TextOperationsWidget::showSearch);
    connect(actionFindNext, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::searchNext);
    connect(actionFindPrev, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::searchPrevious);
    connect(actionReplace, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::showReplace);
    connect(actionGotoLine, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::showGotoLine);

    connect(actionCapitalize, &QAction::triggered, this, &qmdiEditor::transformBlockToUpper);
    connect(actionLowerCase, &QAction::triggered, this, &qmdiEditor::transformBlockToLower);
    connect(actionChangeCase, &QAction::triggered, this, &qmdiEditor::transformBlockCase);
    connect(actionToggleHeader, &QAction::triggered, this, &qmdiEditor::toggleHeaderImpl);

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
    addAction(actionToggleHeader);

    // default is control+b - which we want to use for build
    textEditor->toggleBookmarkAction()->setShortcut(QKeySequence());
}

void qmdiEditor::setModificationsLookupEnabled(bool value) {
    fileModifications = value;
    if (!mdiClientFileName().isEmpty()) {
        if (fileModifications) {
            fileSystemWatcher->addPath(fileName);
        } else {
            fileSystemWatcher->removePath(fileName);
        }
    }
}

void qmdiEditor::setEditorFont(const QFont &newFont) {
    textEditor->setFont(newFont);
    operationsWidget->setTextFont(newFont);
}

void qmdiEditor::setEditorHighlighter(QString id) {
    if (this->syntaxLangID == id) {
        return;
    }
    this->syntaxLangID = id;
    textEditor->setHighlighter(id);
}

bool qmdiEditor::isLocalToolbarVisible() const { return toolbar->isVisible(); }

void qmdiEditor::setLocalToolbarVisible(bool state) const { toolbar->setVisible(state); }

void qmdiEditor::setPreviewEnabled(bool enabled) { this->previewButton->setEnabled(enabled); }

void qmdiEditor::setPreviewVisible(bool enabled) { this->previewButton->setChecked(enabled); }

bool qmdiEditor::isPreviewRequested() {
    return this->previewButton->isEnabled() && this->previewButton->isChecked();
}

bool qmdiEditor::isPreviewVisible() const { return this->previewButton->isChecked(); }

void qmdiEditor::setHistoryModel(SharedHistoryModel *model) {
    operationsWidget->setSearchHistory(model);
}

bool qmdiEditor::isMarkDownDocument() const {
    return fileName.endsWith(".md", Qt::CaseInsensitive);
}

bool qmdiEditor::isXPMDocument() const { return fileName.endsWith(".xpm", Qt::CaseInsensitive); }

bool qmdiEditor::isSVGDocument() const { return fileName.endsWith(".svg", Qt::CaseInsensitive); }

bool qmdiEditor::isXMLDocument() const {
    static QRegularExpression regex(R"(.*xml.*\.xml$)", QRegularExpression::CaseInsensitiveOption);
    return regex.match(syntaxLangID).hasMatch();
}

bool qmdiEditor::isJSONDocument() const {
    return syntaxLangID.contains("json", Qt::CaseInsensitive);
}

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

// FIXME: this is becoming a "on modified" function.
void qmdiEditor::updateClientName() {
    auto static const MODIFIED_TEXT = QStringLiteral(" ") + QChar(0x270D); // pencil

    updatePreview();
    if (textEditor->document()->isModified()) {
        if (!mdiClientName.contains(MODIFIED_TEXT)) {
            mdiClientName = getShortFileName() + MODIFIED_TEXT;
            mdiServer->updateClientName(this);
        }
    } else {
        if (mdiClientName.contains(MODIFIED_TEXT)) {
            mdiClientName = getShortFileName();
            mdiServer->updateClientName(this);
        }
    }
}

void qmdiEditor::goTo(int x, int y) {
    if (documentHasBeenLoaded) {
        textEditor->goTo(x, y);
    } else {
        savedState[StateConstants::ROW] = y;
        savedState[StateConstants::COLUMN] = x;
    }
}

void qmdiEditor::focusInEvent(QFocusEvent *event) {

    QWidget::focusInEvent(event);
    textEditor->setFocus();
}

bool qmdiEditor::eventFilter(QObject *watched, QEvent *event) {
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Show:
            handleTabSelected();
            break;
        case QEvent::Hide:
            handleTabDeselected();
            break;
        default:
            break;
        }
    }

    if (watched == textEditor->viewport()) {
        if (event->type() == QEvent::ToolTip) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            handleWordTooltip(helpEvent->pos(), helpEvent->globalPos());
            return true;
        }

        if (event->type() == QEvent::ContextMenu) {
            auto menuEvent = static_cast<QContextMenuEvent *>(event);
            showContextMenu(menuEvent->pos(), menuEvent->globalPos());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void qmdiEditor::handleTabSelected() {
    if (loadingTimer) {
        return;
    }
    loadingTimer = new QTimer(this);
    loadingTimer->setSingleShot(true);
    connect(loadingTimer, &QTimer::timeout, this, &qmdiEditor::loadContent);
    loadingTimer->start(50);
}

void qmdiEditor::handleTabDeselected() {
    if (!loadingTimer || !loadingTimer->isActive()) {
        return;
    }
    loadingTimer->stop();
    delete loadingTimer;
    loadingTimer = nullptr;
}

QSet<QString> qmdiEditor::getTagCompletions(const QString &prefix) {
    QSet<QString> completions;
    auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    if (!pluginManager) {
        return completions;
    }

    // clang-format off
    auto future = pluginManager->handleCommandAsync(GlobalCommands::VariableInfo, {
        {GlobalArguments::RequestedSymbol, prefix},
        {GlobalArguments::FileName, mdiClientFileName()},
        {GlobalArguments::ExactMatch, false}
    });
    // clang-format o,

    auto maxWaitMs = 500;
    auto pollIntervalMs = 10;
    auto waited = 0;
    while (!future.isFinished() && waited < maxWaitMs) {
        QThread::msleep(pollIntervalMs);
        waited += pollIntervalMs;
    }

    if (future.isFinished() && future.isValid()) {
        auto result = future.result();
        if (result.contains(GlobalArguments::Tags)) {
            auto tags = result[GlobalArguments::Tags].toList();
            for (const QVariant &item : std::as_const(tags)) {
                auto const tag = item.toHash();
                auto const name = tag[GlobalArguments::Name].toString();
                if (!name.isEmpty()) {
                    completions.insert(name);
                }
            }
        }
    }
    return completions;
}

void qmdiEditor::handleWordTooltip(const QPoint &localPosition, const QPoint &globalPosition) {
    auto future = getTooltipsForPosition(localPosition);
    auto watcher = new QFutureWatcher<CommandArgs>(this);
    connect(watcher, &QFutureWatcher<CommandArgs>::finished, this,
            [this, watcher, globalPosition]() {
                if (watcher->isFinished() && !watcher->isCanceled()) {
                    auto res = watcher->result();
                    auto tooltip = res[GlobalArguments::Tooltip].toString();
                    if (!tooltip.isEmpty()) {
                        QToolTip::showText(globalPosition, tooltip, textEditor);
                    }
                }
                watcher->deleteLater();
            });
    watcher->setFuture(future);
}

QFuture<CommandArgs> qmdiEditor::getCommandForLocation(const QPoint &localPosition, const QString &cmd)
{
    auto cursor = textEditor->cursorForPosition(localPosition);
    cursor.select(QTextCursor::WordUnderCursor);

    if (cursor.selectedText().isEmpty()) {
        return {};
    }

    auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    if (!pluginManager) {
        return {};
    }

    auto symbol = cursor.selectedText();
    // clang-format off
    auto res = pluginManager->handleCommandAsync(cmd, {
        {GlobalArguments::RequestedSymbol, symbol },
        {GlobalArguments::FileName, mdiClientFileName() },
        {GlobalArguments::ExactMatch, true },
    });
    // clang-format on
    return res;
}

QFuture<CommandArgs> qmdiEditor::getSuggestionsForCurrentWord(const QPoint &localPosition) {
    return getCommandForLocation(localPosition, GlobalCommands::VariableInfo);
}

QFuture<CommandArgs> qmdiEditor::getTooltipsForPosition(const QPoint &localPosition) {
    return getCommandForLocation(localPosition, GlobalCommands::KeywordTooltip);
}

void qmdiEditor::displayBannerMessage(QString message, int time) {
    banner->show();
    ui_banner->label->setText(message);
    m_timerHideout = time;
    QTimer::singleShot(1000, this, SLOT(hideTimer_timeout()));
}

void qmdiEditor::hideBannerMessage() {
    m_timerHideout = 0;
    ui_banner->label->clear();
    banner->hide();
}

void qmdiEditor::newDocument() { loadFile(""); }

bool qmdiEditor::doSave() {
    if (!documentHasBeenLoaded) {
        return true;
    }
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
    fileName = QDir::toNativeSeparators(newFileName);
    mdiClientName = getShortFileName();

    documentHasBeenLoaded = false;
    if (this->fileName.isEmpty()) {
        this->fileName.clear();
        textEditor->clear();
        return true;
    }

    updateClientName();
    return true;
}

bool qmdiEditor::saveFile(const QString &newFileName) {
    auto sl = fileSystemWatcher->directories();
    if (!sl.isEmpty()) {
        fileSystemWatcher->removePaths(sl);
    }

    auto modificationsEnabledState = getModificationsLookupEnabled();
    setModificationsLookupEnabled(false);
    hideBannerMessage();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    auto file = QFile(newFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    auto textStream = QTextStream(&file);
    auto cursor = QTextCursor(textEditor->document());
    auto op = Qutepart::AtomicEditOperation(textEditor);
    auto doc = textEditor->document();
    for (auto block = doc->begin(); block != doc->end();) {
        auto currentText = block.text();

        if (trimSpacesOnSave) {
            auto static RE = QRegularExpression("\\s+$");
            currentText.remove(RE);
            if (currentText != block.text()) {
                cursor.setPosition(block.position());
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                cursor.insertText(currentText);
                block = cursor.block();
            }
        }

        textStream << currentText;
        block = block.next();
        if (block.isValid()) {
            switch (this->endLineStyle) {
            case UnixEndLine:
                textStream << "\n";
                break;
            case WindowsEndLine:
                textStream << "\r\n";
                break;
            case KeepOriginalEndline:
                textStream << originalLineEnding;
                break;
            }
        }
    }
    file.close();
    textEditor->document()->setModified(false);
    updateClientName();

    QApplication::restoreOverrideCursor();

    fileName = newFileName;
    mdiClientName = getShortFileName();
    textEditor->removeModifications();
    fileSystemWatcher->addPath(newFileName);
    setModificationsLookupEnabled(modificationsEnabledState);
    mdiServer->updateClientName(this);
    updateFileDetails();
    saveBackup();
    return true;
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
        documentHasBeenLoaded = false;
        loadContent();
        hideBannerMessage();
    } else if (s == ":forcerw") {
        hideBannerMessage();
        textEditor->setReadOnly(false);
    }
}

void qmdiEditor::toggleHeaderImpl() {
    auto otherFile = getCorrespondingFile(fileName);
    otherFile = QDir::toNativeSeparators(otherFile);
    if (!otherFile.isEmpty()) {
        auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
        if (pluginManager) {
            pluginManager->openFile(otherFile);
        }
    }
}

void qmdiEditor::loadContent() {
    if (documentHasBeenLoaded) {
        return;
    }
    // clear older watches, and add a new one
    auto sl = fileSystemWatcher->directories();
    if (!sl.isEmpty()) {
        fileSystemWatcher->removePaths(sl);
    }

    auto modificationsEnabledState = getModificationsLookupEnabled();
    setModificationsLookupEnabled(false);
    hideBannerMessage();
    textEditor->setReadOnly(false);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFile file;
    auto loadedFromBackup = false;
    if (savedState.contains(StateConstants::UUID)) {
        uid = savedState[StateConstants::UUID].toString();
        file.setFileName(getBackupFileName());
        if (file.open(QIODevice::ReadOnly)) {
            loadedFromBackup = true;
        } else if (file.setFileName(fileName), !file.open(QIODevice::ReadOnly)) {
            QApplication::restoreOverrideCursor();
            return;
        }
    } else {
        file.setFileName(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QApplication::restoreOverrideCursor();
            return;
        }
    }

    QElapsedTimer timer;
    timer.start();
    this->originalLineEnding = getLineEnding(file, originalLineEnding);
    auto textStream = QTextStream(&file);
    textStream.seek(0);

    // Why blocking signals?
    // When loading, (setPlainText()) a signal is emitted,  which triggers the system to believe
    // that the content has been modified. Just don't do this. It will also save some time
    // on loading, since really, signals emitted a this stage are not meaningful.
    textEditor->blockSignals(true);
    textEditor->setPlainText(textStream.readAll());
    file.close();

    QFileInfo fileInfo(fileName);
    auto elapsed = timer.elapsed();

    if (elapsed > 0) {
        // don't spam logs with very fast loads, we don't care about it
        qDebug() << "qmdiEditor::loadContent " << fileName << "loaded in" << elapsed << "mSec";
    }

    fileName = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    if (fileInfo.exists() && !fileInfo.isWritable()) {
        textEditor->setReadOnly(true);
        displayBannerMessage(
            tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and "
               "change the file attributes for write access'>here to force write access.</a>"),
            10);
    }

    textEditor->document()->setModified(loadedFromBackup);
    updateFileDetails();
    setModificationsLookupEnabled(modificationsEnabledState);
    textEditor->removeModifications();
    textEditor->blockSignals(false);
    QApplication::restoreOverrideCursor();
    documentHasBeenLoaded = true;
    updateClientName();
    setState(savedState);

    // TODO - we should remove dependency on qmdiTabWidget
    if (auto tab = dynamic_cast<qmdiTabWidget *>(mdiServer)) {
        emit tab->newClientAdded(this);
    }

    auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    pluginManager->openFile("loaded:" + fileName);
}

void qmdiEditor::chooseHighliter(const QString &newText) {
    auto langInfo = ::Qutepart::chooseLanguage(QString(), newText, {});
    if (langInfo.isValid()) {
        textEditor->setHighlighter(langInfo.id);
    } else {
        textEditor->removeHighlighter();
    }
}

void qmdiEditor::chooseIndenter(const QAction *action) {
    buttonChangeIndenter->setText(action->text());
    auto act = buttonChangeIndenter->menu()->actions();
    auto j = act.indexOf(action);
    if (j > 0) {
        textEditor->setIndentAlgorithm(static_cast<Qutepart::IndentAlg>(j));
    }
}

void qmdiEditor::findText(const QString &text) {
    auto newCursor = textEditor->textCursor();
    auto findOptions = QFlags<QTextDocument::FindFlag>();
    auto c = textEditor->document()->find(text, newCursor, findOptions);

    // c.movePosition(findOptions.testFlag(QTextDocument::FindBackward) ? QTextCursor::End
    textEditor->setTextCursor(c);
}

/**
 * @brief qmdiEditor::updateFileDetails
 *
 * Called when a file is opened, or saved. Then, the "best" indentator, and
 * syntax is chosen, and fix the UI to reflect it (the bottom combo box).
 *
 * Called on save and load.
 */
void qmdiEditor::updateFileDetails() {
    actionSave->setText(tr("&Save (%1)").arg(getShortFileName()));
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
    } else {
        textEditor->removeHighlighter();
        textEditor->setIndentAlgorithm(Qutepart::IndentAlg::INDENT_ALG_NONE);

        auto a = buttonChangeIndenter->menu()->actions().at(Qutepart::IndentAlg::INDENT_ALG_NONE);

        a->setChecked(true);
        buttonChangeIndenter->setText(a->text());

        auto delegate = static_cast<BoldItemDelegate *>(comboChangeHighlighter->itemDelegate());
        delegate->boldItemStr = PLAIN_TEXT_HIGHIGHTER;

        auto i = comboChangeHighlighter->findText(delegate->boldItemStr);
        if (i > 0) {
            comboChangeHighlighter->setCurrentIndex(i);
        }
    }

    auto isCplusplusSource = fileName.endsWith(".h", Qt::CaseInsensitive) ||
                             fileName.endsWith(".hh", Qt::CaseInsensitive) ||
                             fileName.endsWith(".hpp", Qt::CaseInsensitive) ||
                             fileName.endsWith(".cpp", Qt::CaseInsensitive) ||
                             fileName.endsWith(".c", Qt::CaseInsensitive) ||
                             fileName.endsWith(".cc", Qt::CaseInsensitive) ||
                             fileName.endsWith(".c++", Qt::CaseInsensitive) ||
                             fileName.endsWith(".cxx", Qt::CaseInsensitive);
    actionToggleHeader->setEnabled(isCplusplusSource);
}

void qmdiEditor::updateIndenterMenu() {
    auto langInfo = ::Qutepart::chooseLanguage(QString(), QString(), this->fileName);
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

void qmdiEditor::updatePreview() {
    if (!this->previewButton->isChecked()) {
        return;
    }

    if (isMarkDownDocument()) {
        textPreview->previewText(mdiClientFileName(), textEditor->toPlainText(),
                                 TextPreview::Markdown);
    } else if (isSVGDocument()) {
        textPreview->previewText(mdiClientFileName(), textEditor->toPlainText(), TextPreview::SVG);
    } else if (isXPMDocument()) {
        textPreview->previewText(mdiClientFileName(), textEditor->toPlainText(), TextPreview::XPM);
    } else if (isJSONDocument()) {
        textPreview->previewText(mdiClientFileName(), textEditor->toPlainText(), TextPreview::JSON);
    } else if (isXMLDocument()) {
        textPreview->previewText(mdiClientFileName(), textEditor->toPlainText(), TextPreview::XML);
    }
}

void qmdiEditor::autoSave() {
    if (textEditor->document()->isModified()) {
        saveBackup();
    }
}

QString qmdiEditor::getBackupFileName() const {
    if (uid.isEmpty()) {
        return {};
    }

    auto backupPath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups/";
    QDir().mkpath(backupPath);
    return backupPath + uid + ".bak";
}

void qmdiEditor::saveBackup() {
    auto file = QFile(getBackupFileName());
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << textEditor->toPlainText();
    }
}

void qmdiEditor::loadBackup() {
    auto file = QFile(getBackupFileName());
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        textEditor->setPlainText(in.readAll());
    }
}

void qmdiEditor::deleteBackup() {
    auto file = QFile(getBackupFileName());
    if (file.exists()) {
        file.remove();
    }
}
