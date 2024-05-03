/**
 * \file main.cpp
 * \brief Entry point of application - QtEdit4
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 */

#include "pluginmanager.h"
#include <QApplication>

#include "plugins/help/help_plg.h"
#include "plugins/texteditor/texteditor_plg.h"
// #include "plugins/systembrowser/systembrowser_plg.h"
#include "plugins/ProjectManager/ProjectManagerPlg.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PluginManager pluginManager;

    pluginManager.setFileSettingsManager("qtedit4.ini");

    // 	pluginManager.addPlugin( new RichTextPlugin );
    pluginManager.addPlugin(new TextEditorPlugin);
    //	pluginManager.addPlugin( new FSBrowserPlugin );
    pluginManager.addPlugin(new HelpPlugin);
    pluginManager.addPlugin(new ProjectManagerPlugin);
    pluginManager.updateGUI();

    // start the application
    pluginManager.restoreSettings();
    pluginManager.show();
    return app.exec();
}
