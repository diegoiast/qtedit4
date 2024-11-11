/**
 * \file qmdieditor
 * \brief Implementation of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
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
#include <pluginmanager.h>

#include "../plugins/texteditor/thememanager.h"

#include "qmdieditor.h"
#include "qmdiserver.h"

#include "widgets/textoperationswidget.h"
#include "widgets/textpreview.h"
#include "widgets/ui_bannermessage.h"

#define PLAIN_TEXT_HIGHIGHTER "Plain text"

auto static getCorrespondingFile(const QString &fileName) -> QString {
    static const QStringList cExtensions = {"c", "cpp", "cxx", "cc", "c++"};
    static const QStringList headerExtensions = {"h", "hpp", "hh"};

    auto fileInfo = QFileInfo(fileName);
    auto baseName = fileInfo.baseName();    // Get the base name without extension
    auto dirPath = fileInfo.absolutePath(); // Get the directory path

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
    auto l = QList<QString>();
    l.append(PLAIN_TEXT_HIGHIGHTER);
    l.append(languageNameToXmlFileName.keys());
    return l;
}
} // namespace Qutepart

qmdiEditor::qmdiEditor(QWidget *p, Qutepart::ThemeManager *themes)
    : QWidget(p), themeManager(themes) {
    textEditor = new Qutepart::Qutepart(this);
    operationsWidget = new TextOperationsWidget(textEditor);
    mdiClientName = tr("NO NAME");
    fileSystemWatcher = new QFileSystemWatcher(this);
    QToolBar *toolbar = new QToolBar(this);
    QVBoxLayout *layout = new QVBoxLayout(this);

    setupActions();
    toolbar->addWidget(comboChangeHighlighter);
    toolbar->addWidget(buttonChangeIndenter);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    QWidget *spacer = new QWidget(this);
    QLabel *staticLabel = new QLabel("", toolbar);

    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);
    toolbar->addWidget(staticLabel);
    toolbar->addWidget(previewButton);

    textPreview = new TextPreview(this);
    textPreview->setVisible(false);

    splitter->addWidget(textEditor);
    splitter->addWidget(textPreview);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(splitter);
    layout->addWidget(toolbar);

    connect(textEditor, &QPlainTextEdit::cursorPositionChanged, this, [this, staticLabel]() {
        QTextCursor cursor = textEditor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;
        staticLabel->setText(QString("%1:%2").arg(line).arg(column));
    });

    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &qmdiEditor::on_fileChanged);
    fileModifications = true;

    QFont monospacedFont = this->font();
    monospacedFont.setPointSize(DEFAULT_EDITOR_FONT_SIZE);
    monospacedFont.setFamily(DEFAULT_EDITOR_FONT);
    setEditorFont(monospacedFont);
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
    menus["&Edit"]->addAction(actionFindMatchingBracket);

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
    comboChangeHighlighter = new QComboBox(this);
    buttonChangeIndenter = new QToolButton(this);
    previewButton = new QPushButton(this);
    previewButton->setText(tr("Preview"));
    // previewButton->setEnabled(false);
    previewButton->setFlat(true);
    previewButton->setCheckable(true);

    auto updatePreview = [=]() {
        if (!this->previewButton->isChecked()) {
            return;
        }

        if (mdiClientName.endsWith(".md", Qt::CaseInsensitive)) {
            textPreview->previewText(textEditor->toPlainText(), TextPreview::Markdown);
        } else if (mdiClientName.endsWith(".svg", Qt::CaseInsensitive)) {
            textPreview->previewText(textEditor->toPlainText(), TextPreview::SVG);
        } else if (mdiClientName.endsWith(".xpm", Qt::CaseInsensitive)) {
            textPreview->previewText(textEditor->toPlainText(), TextPreview::XPM);
        } else if (comboChangeHighlighter->currentText().contains("JSON", Qt::CaseInsensitive)) {
            textPreview->previewText(textEditor->toPlainText(), TextPreview::JSON);
        } else if (comboChangeHighlighter->currentText().contains("XML", Qt::CaseInsensitive)) {
            textPreview->previewText(textEditor->toPlainText(), TextPreview::XML);
        }
    };

    connect(previewButton, &QAbstractButton::toggled, this, [=](bool toggled) {
        this->textPreview->setVisible(toggled);
        if (toggled) {
            updatePreview();
        }
    });

    connect(textEditor, &QPlainTextEdit::textChanged, this, updatePreview);

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
    actionCopyFileName = new QAction(tr("Copy filename to clipboard"), this);
    actionCopyFilePath = new QAction(tr("Copy full path to clipboard"), this);

    actionCapitalize = new QAction(tr("Change to &capital letters"), this);
    actionLowerCase = new QAction(tr("Change to &lower letters"), this);
    actionChangeCase = new QAction(tr("Change ca&se"), this);
    actionFindMatchingBracket = new QAction(tr("Find matching bracket"), this);
    actionToggleHeader = new QAction(tr("Toggle header/implementation"), this);

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
    actionFindMatchingBracket->setObjectName("qmdiEditor::ctionFindMatchingBracket");
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
        c->setText(getFileName());
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
            &TextOperationsWidget::searchPrev);
    connect(actionReplace, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::showReplace);
    connect(actionGotoLine, &QAction::triggered, operationsWidget,
            &TextOperationsWidget::showGotoLine);

    connect(actionCapitalize, &QAction::triggered, this, &qmdiEditor::transformBlockToUpper);
    connect(actionLowerCase, &QAction::triggered, this, &qmdiEditor::transformBlockToLower);
    connect(actionChangeCase, &QAction::triggered, this, &qmdiEditor::transformBlockCase);
    connect(actionFindMatchingBracket, &QAction::triggered, this, &qmdiEditor::gotoMatchingBracket);
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
    addAction(actionFindMatchingBracket);
    addAction(actionToggleHeader);

    // not implemented yet in QutePart
    actionFindMatchingBracket->setEnabled(false);

    // default is control+b - which we want to use for build
    textEditor->toggleBookmarkAction()->setShortcut(QKeySequence());
}

void qmdiEditor::setModificationsLookupEnabled(bool value) {
    fileModifications = value;
    if (!fileName.isEmpty()) {
        if (fileModifications) {
            fileSystemWatcher->addPath(fileName);
        } else {
            fileSystemWatcher->removePath(fileName);
        }
    }
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

void qmdiEditor::focusInEvent(QFocusEvent *event) {
    QWidget::focusInEvent(event);
    textEditor->setFocus();
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
    auto sl = fileSystemWatcher->directories();
    if (!sl.isEmpty()) {
        fileSystemWatcher->removePaths(sl);
    }

    auto modificationsEnabledState = getModificationsLookupEnabled();
    setModificationsLookupEnabled(false);
    hideBannerMessage();
    textEditor->setReadOnly(false);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (!newFileName.isEmpty()) {
        auto file = QFile(newFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QApplication::restoreOverrideCursor();
            return false;
        }

        this->originalLineEndig = getLineEnding(file);
        auto textStream = QTextStream(&file);
        textStream.seek(0);
        textEditor->setPlainText(textStream.readAll());

        QFileInfo fileInfo(file);
        file.close();

        this->fileName = fileInfo.absoluteFilePath();
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
                textStream << originalLineEndig;
                break;
            }
        }
    }
    file.close();
    textEditor->document()->setModified(false);

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

void qmdiEditor::toggleHeaderImpl() {
    auto otherFile = getCorrespondingFile(getFileName());
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
    textEditor->setIndentAlgorithm(static_cast<Qutepart::IndentAlg>(j));
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
