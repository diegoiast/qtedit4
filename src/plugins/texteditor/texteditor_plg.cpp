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

#if defined(WIN32)
    auto installPrefix = QCoreApplication::applicationDirPath();
#else
    auto installPrefix = QCoreApplication::applicationDirPath() + "/..";
#endif

    connect(myNewActions, SIGNAL(triggered(QAction *)), this, SLOT(fileNew(QAction *)));
}

TextEditorPlugin::~TextEditorPlugin() {}

void TextEditorPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "This plugin gives a QtSourceView based text editor");
}

QWidget *TextEditorPlugin::getConfigDialog() { return nullptr; }

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
    auto editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer));
    auto loaded = editor->loadFile(fileName);
    mdiServer->addClient(editor);
    editor->goTo(x, y);
    return loaded;
    Q_UNUSED(z);
}

void TextEditorPlugin::navigateFile(qmdiClient *client, int x, int y, int z) {
    auto *editor = dynamic_cast<qmdiEditor *>(client);
    if (!editor) {
        return;
    }
    editor->goTo(x, y);
    Q_UNUSED(z);
}

void TextEditorPlugin::getData() {}

void TextEditorPlugin::setData() {}

void TextEditorPlugin::fileNew(QAction *) {
    qmdiEditor *editor = new qmdiEditor(dynamic_cast<QMainWindow *>(mdiServer));
    mdiServer->addClient(editor);
}
