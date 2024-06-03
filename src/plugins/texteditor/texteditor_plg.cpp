#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QStringList>
#include <QUrl>

#include <qmdiactiongroup.h>
#include <qmdiserver.h>

#include <qsvsh/qsvcolordef.h>
#include <qsvsh/qsvcolordeffactory.h>
#include <qsvsh/qsvlangdeffactory.h>
#include <qsvsh/qsvsyntaxhighlighter.h>
#include <qsvte/qsvsyntaxhighlighterbase.h>

#include "qmdieditor.h"
#include "texteditor_plg.h"

class MyHighlighter : public QsvSyntaxHighlighter, public QsvSyntaxHighlighterBase {
  public:
    MyHighlighter(QTextDocument *parent) : QsvSyntaxHighlighter(parent) {
        setMatchBracketList("{}()[]''\"\"");
    }

    virtual void highlightBlock(const QString &text) override;

    virtual void toggleBookmark(QTextBlock &block) override;

    virtual void removeModification(QTextBlock &block) override;

    virtual void setBlockModified(QTextBlock &block, bool on) override;

    virtual bool isBlockModified(QTextBlock &block) override;

    virtual bool isBlockBookmarked(QTextBlock &block) override;

    virtual Qate::BlockData::LineFlags getBlockFlags(QTextBlock &block) override;

    virtual void clearMatchData(QTextBlock &block) override;

    virtual void addMatchData(QTextBlock &block, Qate::MatchData m) override;

    virtual QList<Qate::MatchData> getMatches(QTextBlock &block) override;

    virtual QTextBlock getCurrentBlockProxy() override;

    Qate::BlockData *getBlockData(QTextBlock &block);
};

void MyHighlighter::highlightBlock(const QString &text) {
    QsvSyntaxHighlighterBase::highlightBlock(text);
    QsvSyntaxHighlighter::highlightBlock(text);
}

void MyHighlighter::toggleBookmark(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return;
    }
    data->toggleBookmark();
}

void MyHighlighter::removeModification(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return;
    }
    data->m_isModified = false;
}

void MyHighlighter::setBlockModified(QTextBlock &block, bool on) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return;
    }
    data->m_isModified = on;
}

bool MyHighlighter::isBlockModified(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return false;
    }
    return data->m_isModified;
}

bool MyHighlighter::isBlockBookmarked(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return 0;
    }
    return data->isBookmark();
}

Qate::BlockData::LineFlags MyHighlighter::getBlockFlags(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return Qate::BlockData::LineFlag::Empty;
    }
    return data->m_flags;
}

void MyHighlighter::clearMatchData(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return;
    }
}

void MyHighlighter::addMatchData(QTextBlock &block, Qate::MatchData m) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return;
    }
    data->matches << m;
}

QList<Qate::MatchData> MyHighlighter::getMatches(QTextBlock &block) {
    Qate::BlockData *data = getBlockData(block);
    if (data == nullptr) {
        return QList<Qate::MatchData>();
    }
    return data->matches;
}

QTextBlock MyHighlighter::getCurrentBlockProxy() { return currentBlock(); }

Qate::BlockData *MyHighlighter::getBlockData(QTextBlock &block) {
    QTextBlockUserData *userData = block.userData();
    Qate::BlockData *blockData = nullptr;

    if (userData == nullptr) {
        blockData = new Qate::BlockData();
        block.setUserData(blockData);
    } else {
        blockData = dynamic_cast<Qate::BlockData *>(userData);
    }
    return blockData;
}

TextEditorPlugin::TextEditorPlugin() {
    name = tr("Text editor plugin - based on QtSourceView");
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

#if defined(WIN32)
    auto installPrefix = QCoreApplication::applicationDirPath();
#else
    auto installPrefix = QCoreApplication::applicationDirPath() + "/..";
#endif
    editorColors = new QsvColorDefFactory(installPrefix + "/share/colors/kate.xml");
    QsvLangDefFactory::getInstanse()->loadDirectory(installPrefix + "/share/langs/");

    connect(myNewActions, SIGNAL(triggered(QAction *)), this, SLOT(fileNew(QAction *)));
}

TextEditorPlugin::~TextEditorPlugin() {}

void TextEditorPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "This plugin gives a QtSourceView based text editor");
}

QWidget *TextEditorPlugin::getConfigDialog() { return NULL; }

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

bool TextEditorPlugin::openFile(const QString fileName, int x, int y, int z) {
    qmdiEditor *editor = new qmdiEditor(fileName, dynamic_cast<QMainWindow *>(mdiServer));

    auto langDefinition =
        QsvLangDefFactory::getInstanse()->getHighlight(editor->mdiClientFileName());
    auto highlighter = new MyHighlighter(editor->document());
    highlighter->setColorsDef(editorColors);
    highlighter->setHighlight(langDefinition);
    highlighter->rehighlight();
    editor->setHighlighter(highlighter);
    editor->removeModifications();

    QPalette p(editor->palette());
    QsvColorDef dsNormal = editorColors->getColorDef("dsNormal");
    if (dsNormal.getBackground().isValid()) {
        p.setColor(QPalette::Base, dsNormal.getBackground());
        editor->setPalette(p);
    }

    mdiServer->addClient(editor);

    editor->gotoLine(x, y);
    // TODO
    // 1) move the cursor as specified in the parameters
    // 2) return false if the was was not open for some reason
    return true;
    Q_UNUSED(z);
}

void TextEditorPlugin::navigateFile(qmdiClient *client, int x, int y, int z) {
    auto *editor = dynamic_cast<qmdiEditor *>(client);
    if (!editor) {
        return;
    }
    editor->gotoLine(x, y);
    Q_UNUSED(z);
}

void TextEditorPlugin::getData() {}

void TextEditorPlugin::setData() {}

void TextEditorPlugin::fileNew(QAction *) {
    qmdiEditor *editor = new qmdiEditor(tr("NO NAME"), dynamic_cast<QMainWindow *>(mdiServer));
    auto *highlighter = new MyHighlighter(editor->document());
    editor->setHighlighter(highlighter);
    mdiServer->addClient(editor);
}
