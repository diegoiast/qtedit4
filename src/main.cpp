/**
 * \file main.cpp
 * \brief Entry point of application - QtEdit4 
 * \author Diego Iastrubni elcuco@kde.org
 * License GPL 2008
 */

#include <QApplication>
#include <pluginmanager.h>
#include "plugins/help/help_plg.h"
#include "plugins/texteditor/texteditor_plg.h"
#include "plugins/systembrowser/systembrowser_plg.h"

int main( int argc, char *argv[] )
{
	QApplication app( argc, argv );
	PluginManager pluginManager;
	
// 	pluginManager.addPlugin( new RichTextPlugin );
	pluginManager.addPlugin( new TextEditorPlugin );
	pluginManager.addPlugin( new FSBrowserPlugin );
	pluginManager.addPlugin( new HelpPlugin );
	pluginManager.updateGUI();
	
	pluginManager.show();
	return app.exec();
}

// kate: space-indent off; tab-indent on; tab-width 6; indent-width 6; mixedindent off; indent-mode cstyle;
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
