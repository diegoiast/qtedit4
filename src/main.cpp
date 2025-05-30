/**
 * \file main.cpp
 * \brief Entry point of application - QtEdit4
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QToolButton>
#include <widgets/qmdiSplitTab.h>

#include "pluginmanager.h"
#include "plugins/CTags/CTagsPlugin.hpp"
#include "plugins/ProjectManager/ProjectManagerPlg.h"
#include "plugins/SplitTabsPlugin/SplitTabsPlugin.hpp"
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/hexviewer/hexviewer_plg.h"
#include "plugins/imageviewer/imageviewer_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

#define USE_SPLIT

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(qutepart_syntax_files);
    Q_INIT_RESOURCE(qutepart_theme_data);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("qtedit4");
    QCoreApplication::setApplicationVersion("0.0.10");

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

    // On bare-bones Linux installs, Windows or OSX, we might not have a freedesktop
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

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(app.tr("files"), app.tr("Files to open."), "[files...]");
    parser.process(app);

    PluginManager pluginManager;
    auto filePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    auto iniFilePath = filePath + "/qtedit4.ini";
    auto textEditorPlugin = new TextEditorPlugin;
    auto windowIcon = QIcon(":qtedit4.ico");

    pluginManager.setWindowTitle("qtedit4");
    pluginManager.setWindowIcon(windowIcon);
    pluginManager.setFileSettingsManager(iniFilePath);

#if defined(USE_SPLIT)
    pluginManager.addPlugin(new SplitTabsPlugin(textEditorPlugin));
#endif
    pluginManager.addPlugin(textEditorPlugin);
    pluginManager.addPlugin(new FileSystemBrowserPlugin);
    pluginManager.addPlugin(new HelpPlugin);
    pluginManager.addPlugin(new ProjectManagerPlugin);
    pluginManager.addPlugin(new ImageViewrPlugin);
    pluginManager.addPlugin(new HexViewrPlugin);
    pluginManager.addPlugin(new CTagsPlugin);
    pluginManager.updateGUI();
    pluginManager.hidePanels(Qt::BottomDockWidgetArea);

    pluginManager.restoreSettings();
    pluginManager.show();

#if !defined(USE_SPLIT)
    pluginManager.connect(&pluginManager, &PluginManager::newFileRequested,
                          [textEditorPlugin]() { textEditorPlugin->fileNew(); });
#endif

    pluginManager.openFiles(parser.positionalArguments());
    return app.exec();
}
