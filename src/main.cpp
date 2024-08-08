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
#include "plugins/ProjectManager/kitdetector.h"
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setApplicationName("qtedit4");
    QApplication app(argc, argv);

    auto aa = KitDetector::findCompilers();
    for (auto a : aa) {
        qDebug() << "- " << a.comment << "at" << a.compiler_path;
    }
    auto qtqt = KitDetector::findQtVersions();
    for (auto a : qtqt) {
        qDebug() << "- " << a.comment << "at" << a.compiler_path;
    }
    return 0;

#if defined(WIN32)
    // default style on windows is ugly and unusable.
    // lets fallback to something more usable for us
    app.setStyle("windowsvista");
    auto needsIcons = true;
    auto iconsPath = "/share/icons";
#else
    auto needsIcons = QIcon::fromTheme(QIcon::ThemeIcon::GoNext).isNull();
    auto iconsPath = "/../share/icons";
#endif

    // On bare bones Linux installs, Windows or OSX,we might now have freedesktop
    // icons thus - we use our bundled icons.
    if (needsIcons) {
        auto base = QDir(QCoreApplication::applicationDirPath() + iconsPath).absolutePath();
        // clang-format off
        auto paths = QIcon::fallbackSearchPaths()
                     << base + "/breeze/actions/16"
                     << base + "/breeze/actions/22"
                     << base + "/breeze/actions/32";
        // clang-format on
        QIcon::setFallbackSearchPaths(paths);
        QIcon::setFallbackThemeName("Breeze");
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
