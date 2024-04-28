/**
 * \file main6.cpp
 * \brief Entry point of the 6st demo
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License GPL 2 or 3
 */

// $Id$

#include <QApplication>
#include <QStatusBar>

#include "pluginmanager.h"
#include "plugins/editor/editor_plg.h"
#include "plugins/help/help_plg.h"
#include "plugins/richtext/richtext_plg.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PluginManager pluginManager;
    // pluginManager.setNativeSettingsManager( "Diego", "PluginManager" );
    pluginManager.setFileSettingsManager("plugin-demo.ini");

    // load a set of default plugins
    pluginManager.addPlugin(new HelpPlugin);
    pluginManager.addPlugin(new EditorPlugin);
    pluginManager.addPlugin(new RichTextPlugin);
    pluginManager.updateGUI();

    // start the application
    pluginManager.restoreSettings();
    pluginManager.statusBar()->showMessage(
        QT_TR_NOOP("Welcome - feel free to configure the GUI to your needs"), 5000);
    // pluginManager.show();

    return app.exec();
}
