/**
 * \file systembrowser_plg.cpp
 * \brief Implementation of the system browser plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FSBrowserPlugin
 */

#include <QAction>
#include <QDockWidget>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QStringList>
#include <QTreeView>
#include <QUrl>

#include <pluginmanager.h>
#include <qmdihost.h>
#include <qmdiserver.h>

#include "filesystembrowser.h"
#include "systembrowser_plg.h"

FSBrowserPlugin::FSBrowserPlugin() {
    name = tr("File system browser");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

	m_fsBrowser	= NULL;
}

FSBrowserPlugin::~FSBrowserPlugin() {}

void FSBrowserPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "A file system browser plugin");
}

void FSBrowserPlugin::on_client_merged(qmdiHost *host) {
    if (m_dockWidget) {
        return;
    }

    QMainWindow *window = dynamic_cast<QMainWindow *>(host);
    m_dockWidget = new QDockWidget(window);
    m_fsBrowser = new FileSystemBrowser(m_dockWidget);

    m_dockWidget->setObjectName("m_dockWidget");
    m_dockWidget->setWindowTitle(tr("File system"));
    m_dockWidget->setWidget(m_fsBrowser);
    window->addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget);

    connect(m_fsBrowser->getTreeView(), SIGNAL(activated(QModelIndex)), this,
            SLOT(on_fileClick(QModelIndex)));
    connect(m_fsBrowser->getListView(), SIGNAL(activated(QModelIndex)), this,
            SLOT(on_fileClick(QModelIndex)));
}

void FSBrowserPlugin::on_client_unmerged(qmdiHost *host) {
    delete (m_dockWidget);
    m_dockWidget = NULL;
    m_fsBrowser = NULL;

    Q_UNUSED(host);
}

void FSBrowserPlugin::on_fileClick(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }

    if (!m_fsBrowser) {
        return;
    }

    if (!m_fsBrowser->getDirModel()) {
        return;
    }

    if (!m_fsBrowser->getDirModel()->fileInfo(index).isFile()) {
        return;
    }

    PluginManager *pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    if (pluginManager) {
        pluginManager->openFile(m_fsBrowser->getDirModel()->fileInfo(index).filePath());
    }
}
