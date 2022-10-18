/**
 * \file help_plg.cpp
 * \brief Implementation of the help system plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see HelpPlugin
 */

#include <QMainWindow>
#include <QMessageBox>
#include <QAction>

#include "help_plg.h"
#include "qmdiserver.h"

HelpPlugin::HelpPlugin()
{
	name = tr("Help system browser");
	author = tr("Diego Iastrubni <diegoiast@gmail.com>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = false;

	actionAbout = new QAction( tr("&About"), this );
	connect( actionAbout, SIGNAL(triggered()), this, SLOT(on_actionAbout_triggered()));

	menus["&Help"]->addAction( actionAbout );
}

HelpPlugin::~HelpPlugin()
{
}

void	HelpPlugin::showAbout()
{
	QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "A file system browser plugin" );
}

void	HelpPlugin::on_actionAbout_triggered()
{
	QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "QtEdit4 - a text editor" );
}
