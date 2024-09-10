#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QUrl>

#include <qmdiactiongroup.h>
#include <qmdiserver.h>

#include "qmdieditor.h"
#include "texteditor_plg.h"

TextEditorPlugin::TextEditorPlugin() {
    name = tr("Text editor plugin - based on QutePart");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    actionNewFile = new QAction(tr("New blank file"), this);
    actionNewCPP = new QAction(tr("New source"), this);
    actionNewHeader = new QAction(tr("New header"), this);
    myNewActions = new QActionGroup(this);
    myNewActions->addAction(actionNewFile);
    myNewActions->addAction(actionNewCPP);
    myNewActions->addAction(actionNewHeader);

    /*
    #if defined(WIN32)
        auto installPrefix = QCoreApplication::applicationDirPath();
    #else
        auto installPrefix = QCoreApplication::applicationDirPath() + "/..";
    #endif
    */

    connect(myNewActions, &QActionGroup::triggered, this, &TextEditorPlugin::fileNew);

    config.pluginName = "Text editor";
    config.description = "Default text editor, based on QutePart";
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Trim spaces"))
                                     .setDescription(tr("Remove spaces from end of lines, on save"))
                                     .setKey(Config::TrimSpacesKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Wrap lines"))
                                     .setDescription(tr("Wrap lines at window size"))
                                     .setKey(Config::WrapLinesKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Auto reload"))
                                     .setDescription(tr("Reload text when file changes on disk"))
                                     .setKey(Config::AutoReloadKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                 .setDisplayName(tr("Smart home/end"))
                                 .setDescription(tr("Move cursor to logical/phisical home/end"))
                                 .setKey(Config::SmartHomeKey)
                                 .setType(qmdiConfigItem::Bool)
                                 .setDefaultValue(true)
                                 .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Show white space"))
                                     .setDescription("Display marks above tabs, and spaces")
                                     .setKey(Config::ShowWhiteKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Draw indentations"))
                                     .setDescription("Display marks on first indentations of lines")
                                     .setKey(Config::ShowIndentationsKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Highlight brackets"))
                                     .setDescription("Draws a background around matching brakcet")
                                     .setKey(Config::HighlightBracketsKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Show line numbers"))
                                     .setDescription("Show or hide the line numbers panel")
                                     .setKey(Config::ShowLineKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Show right margin"))
                                     .setDescription("Shows a a margin at the end of line")
                                     .setKey(Config::MarginKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Wrap index"))
                                     .setDescription(tr("Character at which the margin is drawn"))
                                     .setKey(Config::MarginOffsetKey)
                                     .setType(qmdiConfigItem::UInt16)
                                     .setDefaultValue(80)
                                     .build());
}

TextEditorPlugin::~TextEditorPlugin() {}

void TextEditorPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "This plugin gives a QtSourceView based text editor");
}

QActionGroup *TextEditorPlugin::newFileActions() { return myNewActions; }

QStringList TextEditorPlugin::myExtensions() {
    QStringList s;
    s << tr("Sources", "EditorPlugin::myExtensions") + " (*.c *.cpp *.cxx *.h *.hpp *.hxx *.inc)";
    s << tr("Headers", "EditorPlugin::myExtensions") + " (*.h *.hpp *.hxx *.inc)";
    s << tr("Text files", "EditorPlugin::myExtensions") + " (*.txt)";
    s << tr("Qt project", "EditorPlugin::myExtensions") + " (*.pro *.pri)";
    s << tr("All files", "EditorPlugin::myExtensions") + " (*.*)";

    return s;
}

int TextEditorPlugin::canOpenFile(const QString fileName) {
    QUrl u(fileName);

    // if the scheme is a single line, lets assume this is a windows drive
    if (u.scheme().length() != 1) {
        if ((u.scheme().toLower() != "file") && (!u.scheme().isEmpty())) {
            return -2;
        }
    }

    if (fileName.endsWith(".c", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".cpp", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".cxx", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".h", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".hpp", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".hxx", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".inc", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".pro", Qt::CaseInsensitive)) {
        return 5;
    } else if (fileName.endsWith(".pri", Qt::CaseInsensitive)) {
        return 5;
    } else {
        return 1;
    }
}

bool TextEditorPlugin::openFile(const QString fileName, int x, int y, int zoom) {
    auto editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer));

    // In the future - the zoom, will be used to set state to the lines, if the value is really
    // large. I will assume that font size bigger than 500 is not really existent.
    if (zoom < 1000000) {
        auto f = editor->font();
        f.setPointSize(zoom);
        editor->setFont(f);
    }
    auto loaded = editor->loadFile(fileName);
    mdiServer->addClient(editor);
    editor->goTo(x, y);

    applySettings(editor);
    return loaded;
}

void TextEditorPlugin::navigateFile(qmdiClient *client, int x, int y, int z) {
    auto *editor = dynamic_cast<qmdiEditor *>(client);
    if (!editor) {
        return;
    }
    editor->goTo(x, y);
    Q_UNUSED(z);
}

void TextEditorPlugin::applySettings(qmdiClient *client) {
    auto editor = static_cast<qmdiEditor *>(client);

    if (getConfig().getTrimSpaces()) {
        // editor->
    }

    if (getConfig().getWrapLines()) {
        editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    } else {
        editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    }

    editor->setDrawAnyWhitespace(getConfig().getShowWhite());
    editor->setDrawIndentations(getConfig().getShowIndentations());
    editor->setBracketHighlightingEnabled(getConfig().getHighlightBrackets());
    editor->setLineNumbersVisible(getConfig().getShowLine());
    editor->setSmartHomeEnd(getConfig().getSmartHome());
    editor->setDrawSolidEdge(getConfig().getMargin());
    editor->setLineLengthEdge(getConfig().getMarginOffset());
    editor->repaint();
}

void TextEditorPlugin::configurationHasBeenModified() {
    for (auto i = 0; i < mdiServer->getClientsCount(); i++) {
        auto client = mdiServer->getClient(i);
        auto editor = dynamic_cast<qmdiEditor *>(client);
        if (!editor) {
            continue;
        }
        applySettings(editor);
    }
}

void TextEditorPlugin::fileNew(QAction *) {
    qmdiEditor *editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer));
    mdiServer->addClient(editor);
}
