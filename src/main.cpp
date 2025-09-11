/**
 * \file main.cpp
 * \brief Entry point of application - qtedit4
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QToolButton>

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
    QCoreApplication::setApplicationVersion("0.0.13");

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

#if 1
    auto tintColor = QColor::fromRgb(0x44aa44);
    auto lighterColor = tintColor.lighter(150).name(); // lighter color as hex string
    auto baseColor = tintColor.name();                 // base color as hex string
    auto dockStyle = QString("QDockWidget::title {"
                             "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                             "stop:0 %1, "
                             "stop:1 %2);"
                             "color: white;"
                             "font-weight: bold;"
                             "}")
                         .arg(baseColor, lighterColor);
    qApp->setStyleSheet(dockStyle);
#endif

    auto pal = qApp->palette();
    pal.setColor(QPalette::Highlight, tintColor);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(pal);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(app.tr("files"), app.tr("Files to open."), "[files...]");
    parser.process(app);

    PluginManager pluginManager;
    auto filePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    auto iniFilePath = filePath + "/qtedit4.ini";
    auto textEditorPlugin = new TextEditorPlugin;
    auto windowIcon = QIcon(":qtedit4.ico");

    pluginManager.setWindowTitle(QCoreApplication::applicationName());
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

    pluginManager.connect(&pluginManager, &PluginManager::newFileRequested,
                          [textEditorPlugin]() { textEditorPlugin->fileNew(); });

    pluginManager.openFiles(parser.positionalArguments());
    return app.exec();
}
