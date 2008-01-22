/**
 * \file systembrowser_plg.cpp
 * \brief Implementation of the system browser plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FSBrowserPlugin
 */

#include <QMainWindow>
#include <QUrl>
#include <QMessageBox>
#include <QAction>
#include <QStringList>
#include <QDockWidget>
#include <QDirModel>
#include <QTreeView>

#include <qmdiserver.h>
#include <qmdihost.h>

#include "systembrowser_plg.h"
#include "filesystembrowser.h"

FSBrowserPlugin::FSBrowserPlugin()
{
	name = tr("File system browser");
	author = tr("Diego Iastrubni <elcuco@kde.org>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = false;

	m_dockWidget = NULL;
}

FSBrowserPlugin::~FSBrowserPlugin()
{
}

void	FSBrowserPlugin::showAbout()
{
	QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "A file system browser plugin" );
}

void	FSBrowserPlugin::on_client_merged( qmdiHost* host )
{
	if (m_dockWidget)
		return;

	QMainWindow *window = dynamic_cast<QMainWindow*>(host);
	m_dockWidget	= new QDockWidget(window);
	m_fsBrowser	= new FileSystemBrowser(m_dockWidget); 
	
	m_dockWidget->setWindowTitle( tr("File system") );
	m_dockWidget->setWidget( m_fsBrowser );
	window->addDockWidget( Qt::LeftDockWidgetArea, m_dockWidget );
}

void	FSBrowserPlugin::on_client_unmerged( qmdiHost* host )
{
	delete( m_dockWidget );
	m_dockWidget	= NULL;
	m_fsBrowser	= NULL;
}

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 7; mixedindent off; indent-mode cstyle; 
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
