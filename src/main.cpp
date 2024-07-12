/**
 * \file main.cpp
 * \brief Entry point of application - QtEdit4
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 */

#include <QApplication>
#include <QStandardPaths>

#include "pluginmanager.h"
#include "plugins/ProjectManager/ProjectManagerPlg.h"
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("qtedit4");
    QApplication app(argc, argv);

    PluginManager pluginManager;
    auto filePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    auto iniFilePath = filePath + "/qtedit4.ini";
    pluginManager.setFileSettingsManager(iniFilePath);
    pluginManager.addPlugin(new TextEditorPlugin);
    pluginManager.addPlugin(new FileSystemBrowserPlugin);
    pluginManager.addPlugin(new HelpPlugin);
    pluginManager.addPlugin(new ProjectManagerPlugin);
    pluginManager.updateGUI();
    pluginManager.hideUnusedPanels();
    pluginManager.restoreSettings();
    pluginManager.show();
    return app.exec();
}
