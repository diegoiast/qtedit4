/**
 * \file richtext_plg.cpp
 * \brief Implementation of the RichTextPlugin class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see RichTextPlugin
 */

// $Id: pluginmanager.h 146 2007-04-23 22:45:01Z elcuco $

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QUrl>

#include "iplugin.h"
#include "qmdiserver.h"
#include "richtext_plg.h"
#include "richtextclient.h"

/**
 * \class RichTextPlugin
 *
 *
 *
 *
 *
 */

RichTextPlugin::RichTextPlugin() {
    name = tr("Rich text editor plugin");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    actionNew = new_action(QIcon(":images/filenew.png"), tr("&New HTML file"), this, tr("Ctrl+N"),
                           tr("Create a new file"), SLOT(fileNew()));
    _newFileActions = new QActionGroup(this);
    _newFileActions->addAction(actionNew);

    makeBackups = false;
    showLineNumbers = true;
    makeCurrentLine = true;
    wordWrap = true;
    setData();
}

RichTextPlugin::~RichTextPlugin() {}

void RichTextPlugin::showAbout() {
    QMessageBox::about(NULL, tr("Rich Text Editor plugin"),
                       tr("This plugin brings rich text editing capabilities to the "
                          "application. It can be used as a simple HTML editor"));
}

QWidget *RichTextPlugin::getConfigDialog() { return NULL; }

QActionGroup *RichTextPlugin::newFileActions() { return _newFileActions; }

QStringList RichTextPlugin::myExtensions() {
    QStringList s;
    s << tr("HTML files", "RichTextPlugin::fileOpen: open HTML files") + " (*.html *.html *.xhtml)";

    return s;
}

void RichTextPlugin::getData() {}

void RichTextPlugin::setData() {}

int RichTextPlugin::canOpenFile(const QString fileName) {
    QUrl u(fileName);

    /*
            this code fails on win32
            for example: c:\windows
            scheme = "c:\"

            if ( (u.scheme().toLower() != "file") && (!u.scheme().isEmpty()) )
                    return -1;
    */
    if (fileName.endsWith(".html", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".htm", Qt::CaseInsensitive))
        return 5;
    else if (fileName.endsWith(".xhtml", Qt::CaseInsensitive))
        return 5;
    else
        return -1;
}

bool RichTextPlugin::canCloseClient() {
    qDebug("RichTextPlugin::canCloseClient()");
    return true;
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
bool RichTextPlugin::openFile(const QString fileName, int x, int y, int z) {
    RichTextClient *editor = new RichTextClient(fileName, dynamic_cast<QMainWindow *>(mdiServer));
    //	editor-setMDIclientName( tr("RichText") );
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

void RichTextPlugin::fileNew() {
    if (!mdiServer) {
        qDebug("%s - warning no mdiServer defined", __FUNCTION__);
        return;
    }

    RichTextClient *editor = new RichTextClient(QString(), dynamic_cast<QMainWindow *>(mdiServer));
    editor->mdiClientName = tr("No name");
    editor->setObjectName(editor->mdiClientName);
    mdiServer->addClient(editor);
}
