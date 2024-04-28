/**
 * \file editor_plg.cpp
 * \brief Implementation of the EditorPlugin class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see EditorPlugin
 */

// $Id$

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

#include "editor_plg.h"
#include "iplugin.h"
#include "qexeditor.h"
#include "qexeditor2.h"
#include "qmdiserver.h"

EditorPlugin::EditorPlugin() {
    name = tr("Text editor plugin");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    actionNew = new_action(QIcon(":images/filenew.png"), tr("&New text file"), this, tr("Ctrl+N"),
                           tr("Create a new file"), SLOT(fileNew()));
    _newFileActions = new QActionGroup(this);
    _newFileActions->addAction(actionNew);

    configUI = new QWidget;
    ui.setupUi(configUI);

    makeBackups = false;
    showLineNumbers = true;
    makeCurrentLine = true;
    wordWrap = true;
    setData();
}

EditorPlugin::~EditorPlugin() { delete actionNew; }

void EditorPlugin::showAbout() {
    QMessageBox::about(NULL, tr("Text Editor plugin"),
                       tr("This plugin brings text editing capabilities to the application"));
}

QWidget *EditorPlugin::getConfigDialog() { return configUI; }

QActionGroup *EditorPlugin::newFileActions() { return _newFileActions; }

QStringList EditorPlugin::myExtensions() {
    QStringList s;
    s << tr("Sources", "EditorPlugin::myExtensions") + " (*.c *.cpp *.cxx *.h *.hpp *.hxx *.inc)";
    s << tr("Headers", "EditorPlugin::myExtensions") + " (*.h *.hpp *.hxx *.inc)";
    s << tr("Qt project", "EditorPlugin::myExtensions") + " (*.pro *.pri)";
    s << tr("All files", "EditorPlugin::myExtensions") + " (*.*)";

    return s;
}

void EditorPlugin::getData() {
    makeBackups = ui.cbMakeBackup->isChecked();
    showLineNumbers = ui.cbShowLineNumbers->isChecked();
    makeCurrentLine = ui.cbMarkCurrentLine->isChecked();
    wordWrap = ui.cbWorkWrap->isChecked();
}

void EditorPlugin::setData() {
    ui.cbMakeBackup->setChecked(makeBackups);
    ui.cbShowLineNumbers->setChecked(showLineNumbers);
    ui.cbMarkCurrentLine->setChecked(makeCurrentLine);
    ui.cbWorkWrap->setChecked(wordWrap);

    // emit a signal and notify all editors
    // about the new settings
}

void EditorPlugin::loadConfig(QSettings &settings) {
    settings.beginGroup("EditorPlugin");
    makeBackups = settings.value("makeBackups", makeBackups).toBool();
    showLineNumbers = settings.value("showLineNumbers", showLineNumbers).toBool();
    makeCurrentLine = settings.value("makeCurrentLine", makeCurrentLine).toBool();
    wordWrap = settings.value("wordWrap", wordWrap).toBool();
    makeBackups = settings.value("makeBackups", makeBackups).toBool();
    // editorFont = settings.value( "editorFont", editorFont ).;
    endOfLine = settings.value("endOfLine", endOfLine).toInt();
    settings.endGroup();
}

void EditorPlugin::saveConfig(QSettings &settings) {
    settings.beginGroup("EditorPlugin");
    settings.setValue("makeBackups", makeBackups);
    settings.setValue("showLineNumbers", showLineNumbers);
    settings.setValue("makeCurrentLine", makeCurrentLine);
    settings.setValue("wordWrap", wordWrap);
    settings.setValue("makeBackups", makeBackups);
    settings.setValue("editorFont", editorFont);
    settings.setValue("endOfLine", endOfLine);
    settings.endGroup();
}

int EditorPlugin::canOpenFile(const QString fileName) {
    QUrl u(fileName);

    /*	this code fails on win32
            for example: c:\windows
            scheme = "c:\"
     */
    // if the scheme is a single line, lets assume this is a windows drive
    if (u.scheme().length() != 1)
        if ((u.scheme().toLower() != "file") && (!u.scheme().isEmpty()))
            return -2;

    if (fileName.endsWith(".c", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".cpp", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".cxx", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".h", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".hpp", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".hxx", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".inc", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".pro", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".pri", Qt::CaseInsensitive))
        return 5;
    else
        return 1;
}

/**
 * \brief open a file
 * \param x the row to move the cursor to
 * \param y the line to move the cursor to
 * \param z unused, ignored
 * \return true if the file was opened, and the cursor reached the specified
 * location
 *
 * This function is used to open a file. The \b x and \b y parameters
 * can be used to specify the row/column to move the cursor to. If those
 * parameters have the value -1 the cursor will move to the "0" position
 * of that coordinate.
 *
 * If the file was not open, the function will return false.
 * If the cursor position could not be reached (out of bounds for example)
 * the function will return false.
 * On all other cases, return true to represent that the action was completed
 * without any problems.
 *
 */
bool EditorPlugin::openFile(const QString fileName, int x, int y, int z) {
    QexTextEdit *editor = new QexTextEdit2(fileName, true, dynamic_cast<QMainWindow *>(mdiServer));
    editor->hide();
    mdiServer->addClient(editor);

    // TODO
    // 1) move the cursor as specified in the parameters
    // 2) return false if the was was not open for some reason
    return true;
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(z);
}

void EditorPlugin::fileNew() {
    if (!mdiServer) {
        qDebug("%s - %d : warning no mdiServer defined", __FUNCTION__, __LINE__);
        return;
    }

    QexTextEdit *editor = new QexTextEdit2(QString(), true);
    editor->mdiClientName = tr("No name");
    editor->setObjectName(editor->mdiClientName);
    mdiServer->addClient(editor);
}
