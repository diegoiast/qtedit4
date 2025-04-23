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
#include "plugins/filesystem/filesystembrowser.h"
#include "plugins/help/help_plg.h"
#include "plugins/hexviewer/hexviewer_plg.h"
#include "plugins/imageviewer/imageviewer_plg.h"
#include "plugins/texteditor/texteditor_plg.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(qutepart_syntax_files);
    Q_INIT_RESOURCE(qutepart_theme_data);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("qtedit4");
    QCoreApplication::setApplicationVersion("0.0.9-alpha1");

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

    // On bare bones Linux installs, Windows or OSX, we might not have a freedesktop
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

#if 1
    auto split = new qmdiSplitTab;
    auto splitAction = new QAction("Split tabs", split);
    split->setButtonProvider(new DefaultButtonsProvider);
    splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));
    QObject::connect(splitAction, &QAction::triggered, splitAction, [split, textEditorPlugin]() {
        split->splitHorizontally();
        textEditorPlugin->fileNew();
    });
    auto moveSplitAction = new QAction("Move editor to new split", split);
    moveSplitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backslash));
    QObject::connect(moveSplitAction, &QAction::triggered, moveSplitAction, [split]() {
        // TODO - this action should be disabled. I need expose the event of tabs modifies
        //        to 3rd parties, and not keep it an internal event.
        if (split->getWigetsCountInCurrentSplit() < 2) {
            return;
        }
        auto w = split->getCurrentWidget();
        split->moveTabToNewSplit(w);
    });
    QObject::connect(split, &qmdiSplitTab::newClientAdded, &pluginManager,
                     &PluginManager::newClientAdded);

    pluginManager.removeBuiltinActions();
    pluginManager.menus["Se&ttings"]->addAction(splitAction);
    pluginManager.menus["Se&ttings"]->addAction(moveSplitAction);
    pluginManager.replaceMdiServer(split);
    pluginManager.addBuiltinActions();
    pluginManager.updateGUI();
#endif
    pluginManager.setWindowTitle("qtedit4");
    pluginManager.setWindowIcon(windowIcon);
    pluginManager.setFileSettingsManager(iniFilePath);
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
                          [&pluginManager, textEditorPlugin, split](QObject *s) {
                              auto tab = qobject_cast<QTabWidget *>(s->parent());
                              auto index = split->findSplitIndex(tab);
                              qDebug() << "sender tab is" << tab << "split" << index;
                              if (!tab || index < 0) {
                                  textEditorPlugin->fileNew();
                                  return;
                              }

                              // we know where exactly to put this
                              auto client = textEditorPlugin->fileNewEditor();
                              auto editor = dynamic_cast<QWidget *>(client);
                              split->addTabToSplit(index, editor, client->mdiClientName,
                                                   client->mdiClientFileName());
                              client->mdiServer = pluginManager.getMdiServer();
                              editor->setFocus();
                          });

    pluginManager.openFiles(parser.positionalArguments());
    return app.exec();
}
