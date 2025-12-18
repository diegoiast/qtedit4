/**
 * \file main.cpp
 * \brief Entry point of application - CodePointer
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>
#include <QToolButton>

#include "GlobalCommands.hpp"
#include "pluginmanager.h"
#include "plugins/CTags/CTagsPlugin.hpp"
#include "plugins/ProjectManager/ProjectManagerPlg.h"
#include "plugins/SplitTabsPlugin/SplitTabsPlugin.hpp"
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/hexviewer/hexviewer_plg.h"
#include "plugins/imageviewer/imageviewer_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

const QString WelcomContent = R"(
# Welcome to CodePointer

CodePointer - an IDE for Rust, Go, C++ and more. The application
will look like a normal text editor, but can

![CodePointer](https://raw.githubusercontent.com/diegoiast/qtedit4/337cd10aab4b123b15dee53c446fd1b7e343dc9a/qtedit4.png)

Some hints for starter:

 * Normal keyboard shortcuts you are used
   to should work (`control+f`, `control+o`, `control+s` and more).
 * To select tabs, press `alt+1` etc.
 * To select/hide/show hide sidebar, press `control+1` (this will open the file
   manager).
 * On the top left, you will find the application menu (shortcut is `alt+m`).
 * You can access the command palette which has all the available commands
   using `control+shift+p`.
 * You can press `alt+control+m` to get a conservative menus+toolbars UI.
 * You can split the editor horizontally by pressing

## Project management

You can also load projects, build and execute them:
 * If you edit a `CMakeLists.txt` or `meson.build` or `cargo.toml` you will be
   prompted to open this file as a project. A new sidebar will be opened with
   the project files.
 * You can also add an "existing project", by choosing a directory.
 * You can choose commands to execute for building, or other tasks relevant
   to this project (configure, build), and you can choose which target
   to run (`control+b` and `control-r`).
 * When building, errors are shown at the bottom.
 * You can execute script files (python, Perl, Bash, PowerShell, etc.), by
   pressing `control+shift+r`
)";

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(qutepart_syntax_files);
    Q_INIT_RESOURCE(qutepart_theme_data);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(CODEPOINTER_APP_NAME);
    QCoreApplication::setApplicationVersion("0.0.16");

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

#if defined(BUILD_DEV)
    auto tintBackgroundColor = QColor::fromRgb(0xFFC107);
#elif defined(BUILD_OFFICIAL)
    auto tintBackgroundColor = QColor::fromRgb(0x44aa44);
#endif

#if !defined(BUILD_CE)
    auto tintTextColor = Qt::white;

    auto lighterColor = tintBackgroundColor.lighter(150).name();
    auto baseColor = tintBackgroundColor.name();
    auto dockStyle = QString("QDockWidget::title {"
                             "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                             "stop:0 %1, "
                             "stop:1 %2);"
                             "color: white;"
                             "font-weight: bold;"
                             "}")
                         .arg(baseColor, lighterColor);
    qApp->setStyleSheet(dockStyle);

    auto pal = qApp->palette();
    pal.setColor(QPalette::Highlight, tintBackgroundColor);
    pal.setColor(QPalette::HighlightedText, tintTextColor);
    qApp->setPalette(pal);
#endif

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(app.tr("files"), app.tr("Files to open."), "[files...]");
    parser.process(app);

    PluginManager pluginManager;
    auto filePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    auto iniFilePath = filePath + QString("/%1.ini").arg(CODEPOINTER_APP_NAME);
    auto windowIcon = QIcon(CODEPOINTER_ICON);
    auto textEditorPlugin = new TextEditorPlugin;
    auto split = new SplitTabsPlugin(textEditorPlugin);

    pluginManager.setWindowTitle(QCoreApplication::applicationName());
    pluginManager.setWindowIcon(windowIcon);
    pluginManager.setFileSettingsManager(iniFilePath);

    pluginManager.addPlugin(split);
    pluginManager.addPlugin(textEditorPlugin);
    pluginManager.addPlugin(new HelpPlugin);
    pluginManager.addPlugin(new CTagsPlugin);
    pluginManager.addPlugin(new FileSystemBrowserPlugin);
    pluginManager.addPlugin(new ProjectManagerPlugin);
    pluginManager.addPlugin(new ImageViewrPlugin);
    pluginManager.addPlugin(new HexViewrPlugin);
    split->setLoadingFinished(false);

    // Those are defaults, restore will override them
    pluginManager.hidePanels(Qt::BottomDockWidgetArea);
    pluginManager.hidePanels(Qt::LeftDockWidgetArea);
    pluginManager.hidePanels(Qt::RightDockWidgetArea);
    pluginManager.actionHideGUI->setChecked(true);
    pluginManager.restoreSettings();
    pluginManager.openFiles(parser.positionalArguments());
    split->setLoadingFinished(true);
    pluginManager.updateGUI();

    if (pluginManager.visibleTabs() == 0) {
        CommandArgs args = {
            {GlobalArguments::FileName, "welcome.md"},
            {GlobalArguments::Content, WelcomContent},
        };
        pluginManager.handleCommandAsync(GlobalCommands::DisplayText, args);
        pluginManager.saveSettings();
    }

    pluginManager.show();
    return app.exec();
}
