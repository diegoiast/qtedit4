/**
 * \file qmdieditor
 * \brief Implementation of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#include <QActionGroup>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextEdit>
#include <QToolBar>
#include <QTreeView>
#include <qmditabwidget.h>

#include <pluginmanager.h>
#include <qmdiserver.h>

#include "plugins/texteditor/thememanager.h"
#include "qmdieditor.h"
#include "widgets/textoperationswidget.h"
#include "widgets/textpreview.h"
#include "widgets/ui_bannermessage.h"

#define PLAIN_TEXT_HIGHIGHTER "Plain text"

auto static is_running_under_gnome() -> bool {
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
}

auto static getCorrespondingFile(const QString &fileName) -> QString {
    auto static const cExtensions = QStringList{"c", "cpp", "cxx", "cc", "c++"};
    auto static const headerExtensions = QStringList{"h", "hpp", "hh"};

    auto fileInfo = QFileInfo(fileName);
    auto baseName = fileInfo.baseName();
    auto dirPath = fileInfo.absolutePath();

    if (cExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
        for (const auto &headerExt : headerExtensions) {
            auto headerFileName = dirPath + "/" + baseName + "." + headerExt;
            if (QFileInfo::exists(headerFileName)) {
                return headerFileName;
            }
        }
    } else if (headerExtensions.contains(fileInfo.suffix(), Qt::CaseInsensitive)) {
        for (const auto &cExt : cExtensions) {
            auto cFileName = dirPath + "/" + baseName + "." + cExt;
            if (QFileInfo::exists(cFileName)) {
                return cFileName;
            }
        }
    }
    return {};
}

auto static getLineEnding(QIODevice &stream) -> QString {
    if (stream.atEnd()) {
        return {};
    }

    auto pos = stream.pos();
    auto ending = QString();
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
    auto toolbar = new QWidget(this);
    auto layout2 = new QHBoxLayout(toolbar);
    auto layout = new QVBoxLayout(this);

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
    fnt.setPointSize(18);
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
    textOperationsMenu->setObjectName("qmdiEditor::textOperationsMenu");
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
    menus["&Edit"]->addAction(textEditor->findMatchingBracketAction());

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

    this->contextMenu.addSeparator();
    this->contextMenu.addAction(actionCopyFileName);
    this->contextMenu.addAction(actionCopyFilePath);

    this->installEventFilter(this);

#if defined(WIN32)
    originalLineEnding = "\r\n";
#else
    originalLineEnding = "\n";
#endif
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

void qmdiEditor::setupActions() {
    buttonChangeIndenter = new QToolButton(this);
    comboChangeHighlighter = new QComboBox(this);
    previewButton = new QPushButton(this);
    previewButton->setText(tr("Preview"));
    previewButton->setFlat(true);
    previewButton->setCheckable(true);

    connect(previewButton, &QAbstractButton::toggled, this, [=](bool toggled) {
        this->textPreview->setVisible(toggled);
        if (toggled) {
            updatePreview();
        }
    });

    // FIXME - the new syntax for connecting a signal/slot crashes the app, using the old one works
    // connect(textEditor, &QPlainTextEdit::textChanged, this, &qmdiEditor::updatePreview);
    connect(textEditor, SIGNAL(textChanged()), this, SLOT(updatePreview()));

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

    connect(textEditor, &QPlainTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::undoAvailable, actionUndo, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::redoAvailable, actionRedo, &QAction::setEnabled);

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
    connect(actionCopy, &QAction::triggered, textEditor, &QPlainTextEdit::copy);
    connect(actionCut, &QAction::triggered, textEditor, &QPlainTextEdit::cut);
    connect(actionPaste, &QAction::triggered, textEditor, &QPlainTextEdit::paste);
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

void qmdiEditor::setEditorHighlighter(QString id) {
    if (this->syntaxLangID == id) {
        return;
    }
    this->syntaxLangID = id;
    textEditor->setHighlighter(id);
}

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
    return mdiClientName.endsWith(".md", Qt::CaseInsensitive);
}

bool qmdiEditor::isXPMDocument() const {
    return mdiClientName.endsWith(".xpm", Qt::CaseInsensitive);
}

bool qmdiEditor::isSVGDocument() const {
    return mdiClientName.endsWith(".svg", Qt::CaseInsensitive);
}

bool qmdiEditor::isXMLDocument() const {
    QRegularExpression regex(R"(.*xml.*\.xml$)", QRegularExpression::CaseInsensitiveOption);
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

void qmdiEditor::goTo(int x, int y) {
    if (documentHasBeenLoaded) {
        textEditor->goTo(x, y);
    } else {
        requestedPosition.setX(x);
        requestedPosition.setY(y);
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

    if (fileName.isEmpty()) {
        this->fileName.clear();
        textEditor->clear();
        return true;
    }

    documentHasBeenLoaded = false;
    return true;
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
    QTextCursor cursor(textEditor->document());
    while (block.isValid()) {
        QString s = block.text();

        if (trimSpacesOnSave) {
            s = s.trimmed();
            cursor.setPosition(block.position());
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.insertText(s);
        }

        textStream << s;
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

    QApplication::restoreOverrideCursor();

    this->fileName = newFileName;
    this->mdiClientName = getShortFileName();
    textEditor->removeModifications();
    fileSystemWatcher->addPath(newFileName);
    setModificationsLookupEnabled(modificationsEnabledState);

    auto w = dynamic_cast<QTabWidget *>(this->mdiServer);
    auto i = w->indexOf(this);
    w->setTabText(i, mdiClientName);
    w->setTabToolTip(i, mdiClientFileName());
    updateFileDetails();
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
        loadFile(fileName);
        hideBannerMessage();
    } else if (s == ":forcerw") {
        hideBannerMessage();
        textEditor->setReadOnly(false);
    }
}

void qmdiEditor::toggleHeaderImpl() {
    auto otherFile = getCorrespondingFile(fileName);
    if (!otherFile.isEmpty()) {
        auto pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
        if (pluginManager) {
            pluginManager->openFile(otherFile);
        }
    }
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

    auto file = QFile(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QApplication::restoreOverrideCursor();
        return;
    }

    QElapsedTimer timer;
    timer.start();
    this->originalLineEnding = getLineEnding(file);
    auto textStream = QTextStream(&file);
    textStream.seek(0);
    textEditor->setPlainText(textStream.readAll());
    textEditor->goTo(requestedPosition.x(), requestedPosition.y());

    QFileInfo fileInfo(file);
    file.close();
    qDebug() << "File " << fileName << "loaded in" << timer.elapsed() << "mSec";

    fileName = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    if (!fileInfo.isWritable()) {
        textEditor->setReadOnly(true);
        displayBannerMessage(
            tr("The file is readonly. Click <a href=':forcerw' title='Click here to try and "
               "change the file attributes for write access'>here to force write access.</a>"),
            10);
    }

    updateFileDetails();
    setModificationsLookupEnabled(modificationsEnabledState);
    textEditor->removeModifications();
    QApplication::restoreOverrideCursor();
    documentHasBeenLoaded = true;

    if (auto tab = dynamic_cast<qmdiTabWidget *>(mdiServer)) {
        emit tab->newClientAdded(this);
    }
}
