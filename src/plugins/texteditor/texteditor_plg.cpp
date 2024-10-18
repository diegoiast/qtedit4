#include "thememanager.h"
#include <CommandPaletteWidget/commandpalette.h>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QChar>
#include <QCoreApplication>
#include <QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QUrl>
#include <qmdiactiongroup.h>
#include <qmdihost.h>
#include <qmdiserver.h>

#include "qmdieditor.h"
#include "texteditor_plg.h"

bool isPlainText(const QString &str) {
    if (str.isEmpty()) {
        return true;
    }
    auto textCharCount = 0;
    auto totalCharCount = 0;
    for (const auto &ch : str) {
        totalCharCount++;
        if (ch == '\0') {
            return false;
        }
        if (ch.isLetterOrNumber() || ch.isPunct() || ch.isSpace() || ch.isSymbol() || ch.isMark()) {
            textCharCount++;
        } else if (ch.category() == QChar::Other_Control && ch.unicode() < 32) {
            if (ch == '\n' || ch == '\r' || ch == '\t') {
                textCharCount++;
            }
        }
    }
    auto textRatio = static_cast<double>(textCharCount) / totalCharCount;
    return textRatio > 0.90;
}

TextEditorPlugin::TextEditorPlugin() {
    name = tr("Text editor plugin - based on QutePart");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
    themeManager = new Qutepart::ThemeManager();

    /*
    #if defined(WIN32)
        auto installPrefix = QCoreApplication::applicationDirPath();
    #else
        auto installPrefix = QCoreApplication::applicationDirPath() + "/..";
    #endif
    */

    config.pluginName = tr("Text editor");
    config.description = tr("Default text editor, based on QutePart");
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
                                     .setDescription(tr("Move cursor to logical/physical home/end"))
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

    auto values = QStringList() << tr("Unix end of line") << tr("Windows end of line")
                                << tr("Keep original end of line");
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("End of line style"))
                                     .setDescription(tr("Which line ends to use when saving"))
                                     .setKey(Config::LineEndingSaveKey)
                                     .setType(qmdiConfigItem::OneOf)
                                     .setPossibleValue(values)
                                     .setDefaultValue(EndLineStyle::KeepOriginalEndline)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Show right margin"))
                                     .setDescription("Shows a a margin at the end of line")
                                     .setKey(Config::MarginKey)
                                     .setType(qmdiConfigItem::Bool)
                                     .setDefaultValue(true)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Margin position"))
                                     .setDescription(tr("Character at which the margin is drawn"))
                                     .setKey(Config::MarginOffsetKey)
                                     .setType(qmdiConfigItem::UInt16)
                                     .setDefaultValue(80)
                                     .build());

    QFont monospacedFont = qApp->font();
    monospacedFont.setPointSize(DEFAULT_EDITOR_FONT_SIZE);
    monospacedFont.setFamily(DEFAULT_EDITOR_FONT);

    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Display font"))
                                     .setKey(Config::FontKey)
                                     .setType(qmdiConfigItem::Font)
                                     .setDefaultValue(monospacedFont)
                                     .setValue(monospacedFont)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::ThemeKey)
                                     .setType(qmdiConfigItem::Font)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());
}

TextEditorPlugin::~TextEditorPlugin() {}

void TextEditorPlugin::on_client_merged(qmdiHost *) {
    chooseTheme = new QAction(tr("Choose theme"), getManager());
    menus[tr("Se&ttings")]->addAction(chooseTheme);

    connect(chooseTheme, &QAction::triggered, this, [this]() {
        auto current = getManager()->currentClient();
        auto editor = dynamic_cast<qmdiEditor *>(current);
        if (!editor) {
            return;
        }
        auto langInfo = ::Qutepart::chooseLanguage({}, {}, editor->mdiClientFileName());

        newThemeSelected = false;
        chooseTheme->setDisabled(true);
        auto list = QStringList(tr("System colors"));
        for (auto &t : themeManager->getLoadedFiles()) {
            auto m = themeManager->getThemeMetaData(t);
            list.append(m.name);
        }
        list.sort(Qt::CaseSensitive);

        auto model = new QStringListModel(list, getManager());
        auto p = new CommandPalette(getManager());
        p->setDataModel(model);
        p->show();

        connect(p, &CommandPalette::didHide, this, [this, p, editor, langInfo]() {
            if (!newThemeSelected) {
                editor->setEditorTheme(this->theme);
                editor->setEditorHighlighter(langInfo.id, this->theme);
            } else {
                // apply it globally
                auto newTheme = const_cast<Qutepart::Theme *>(editor->getEditorTheme());
                delete this->theme;
                this->theme = newTheme;
                for (auto i = 0; i < mdiServer->getClientsCount(); i++) {
                    auto client = mdiServer->getClient(i);
                    auto e = dynamic_cast<qmdiEditor *>(client);
                    if (!e) {
                        // current editor already has this enabled
                        continue;
                    }

                    auto langInfo = ::Qutepart::chooseLanguage({}, {}, e->mdiClientFileName());
                    e->setEditorTheme(this->theme);
                    e->setEditorHighlighter(langInfo.id, this->theme);
                }

                auto themeFileName = themeManager->getNameFromDesc(newTheme->metaData.name);
                getConfig().setTheme(themeFileName);
            }
            chooseTheme->setEnabled(true);
            p->deleteLater();
            editor->setFocus();
        });
        connect(p, &CommandPalette::didSelectItem, this,
                [langInfo, editor, this](const QModelIndex index, const QAbstractItemModel *) {
                    auto newTheme = const_cast<Qutepart::Theme *>(editor->getEditorTheme());
                    if (newTheme != this->theme) {
                        delete newTheme;
                        newTheme = nullptr;
                    }
                    auto themeDescription = index.data(Qt::DisplayRole).toString();
                    auto themeFileName = themeManager->getNameFromDesc(themeDescription);
                    auto themeMetaData = themeManager->getThemeMetaData(themeFileName);
                    if (!themeMetaData.name.isEmpty()) {
                        newTheme = new Qutepart::Theme();
                        const_cast<Qutepart::Theme *>(newTheme)->loadTheme(themeFileName);
                    }

                    editor->setEditorTheme(newTheme);
                    editor->setEditorHighlighter(langInfo.id, newTheme);
                });
        connect(p, &CommandPalette::didChooseItem, this, [editor, this]() {
            // don't restore theme on closing - user choose his new theme
            newThemeSelected = true;
        });
    });
}

void TextEditorPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "This plugin gives a QtSourceView based text editor");
}

void TextEditorPlugin::loadConfig(QSettings &settings) {
    IPlugin::loadConfig(settings);
    themeManager->loadFromDir(":/qutepart/themes/");
    qDebug() << "Loaded themes, count=" << themeManager->getLoadedFiles().length();

    auto themeFileName = getConfig().getTheme();
    delete this->theme;
    this->theme = new Qutepart::Theme();
    if (!this->theme->loadTheme(themeFileName)) {
        qDebug() << "Failed loading thene " << themeFileName;
        delete this->theme;
        this->theme = nullptr;
    }
}

QStringList TextEditorPlugin::myExtensions() {
    auto s = QStringList();
    s << tr("Sources", "EditorPlugin::myExtensions") + " (*.c *.cpp *.cxx *.h *.hpp *.hxx *.inc)";
    s << tr("Headers", "EditorPlugin::myExtensions") + " (*.h *.hpp *.hxx *.inc)";
    s << tr("Text files", "EditorPlugin::myExtensions") + " (*.txt)";
    s << tr("Qt project", "EditorPlugin::myExtensions") + " (*.pro *.pri)";
    s << tr("All files", "EditorPlugin::myExtensions") + " (*.*)";
    return s;
}

int TextEditorPlugin::canOpenFile(const QString fileName) {
    if (fileName.isEmpty()) {
        return 5;
    }
    auto u = QUrl(fileName);

    // if the scheme is a single line, lets assume this is a windows drive
    if (u.scheme().length() != 1) {
        if ((u.scheme().toLower() != "file") && (!u.scheme().isEmpty())) {
            return -2;
        }
    }

    static const QStringList extensions = {
        ".c",    ".cpp", ".cxx",  ".cc",     ".h",       ".hpp",    ".hxx",     ".inc",
        ".pro",  ".pri", ".txt",  ".inc",    ".java",    ".js",     ".py",      ".rb",
        ".go",   ".pas", ".bas",  ".swift",  ".mk",      ".bat",    ".sh",      ".md",
        ".user", ".txt", ".text", ".dox",    ".desktop", ".old",    ".bak",     ".mk",
        ".lic",  ".xml", ".json", "license", "readme",   "copying", "Makefile", "Doxyfile"};
    for (const auto &ext : extensions) {
        if (fileName.endsWith(ext, Qt::CaseInsensitive)) {
            return 5;
        }
    }

    if (fileName.startsWith(".")) {
        return 5;
    }

    auto file = QFile(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto in = QTextStream(&file);
        QString firstLines = in.readLine();
        firstLines.append(in.readLine());
        firstLines.append(in.readLine());
        firstLines.append(in.readLine());
        firstLines.append(in.readLine());
        file.close();
        if (isPlainText(firstLines)) {
            return 5;
        }
    }

    return 1;
}

bool TextEditorPlugin::openFile(const QString fileName, int x, int y, int zoom) {
    auto editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer), themeManager);

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

void TextEditorPlugin::applySettings(qmdiEditor *editor) {
    if (getConfig().getWrapLines()) {
        editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    } else {
        editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    }

    auto newFont = QFont();
    newFont.fromString(getConfig().getFont());

    editor->setDrawAnyWhitespace(getConfig().getShowWhite());
    editor->setDrawIndentations(getConfig().getShowIndentations());
    editor->setBracketHighlightingEnabled(getConfig().getHighlightBrackets());
    editor->setLineNumbersVisible(getConfig().getShowLine());
    editor->setSmartHomeEnd(getConfig().getSmartHome());
    editor->setDrawSolidEdge(getConfig().getMargin());
    editor->setLineLengthEdge(getConfig().getMarginOffset());
    editor->endLineStyle = getConfig().getLineEndingSave();
    editor->trimSpacesOnSave = getConfig().getTrimSpaces();
    editor->setEditorFont(newFont);
    editor->repaint();

    if (this->theme != editor->getEditorTheme()) {
        auto langInfo = ::Qutepart::chooseLanguage({}, {}, editor->mdiClientFileName());
        editor->setEditorTheme(this->theme);
        editor->setEditorHighlighter(langInfo.id, this->theme);
    }
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

void TextEditorPlugin::fileNew() {
    auto editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer), themeManager);
    mdiServer->addClient(editor);
}
