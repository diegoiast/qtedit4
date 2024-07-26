/**
 * \file main.cpp
 * \brief Entry point of application - QtEdit4
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 */

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>

#include "pluginmanager.h"
#include "plugins/ProjectManager/ProjectManagerPlg.h"
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("qtedit4");
    QApplication app(argc, argv);

#if defined(WIN32)
    // default style on windows is ugly and unusable.
    // lets fallback to something more usable for us
    app.setStyle("windowsvista");

    // Force using internal icon set on windows.
    auto needsIcons = true;
#else
    auto needsIcons = !QIcon::hasThemeIcon(QIcon::ThemeIcon::ApplicationExit);
#endif

    if (needsIcons) {
        // On bare bones Linux installs, Windows or OSX,we might now have freedesktop icons
        // thus - we use our bundled icons.
#if defined(WIN32)
        auto base = QDir(QCoreApplication::applicationDirPath() + "icons").absolutePath();
#else
        auto base = QDir(QCoreApplication::applicationDirPath() + "/../share/icons").absolutePath();
#endif
        auto paths = QIcon::fallbackSearchPaths() << base;
        QIcon::setFallbackSearchPaths(paths);
        QIcon::setThemeName("Breeze");
        qDebug() << "No icons found, using our own. Icons search path" << paths;
    }

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
