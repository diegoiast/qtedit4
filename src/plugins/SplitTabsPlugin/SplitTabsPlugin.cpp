#include <iplugin.h>
#include <pluginmanager.h>

#include "SplitTabsPlugin.hpp"
#include "plugins/texteditor/texteditor_plg.h"
#include "widgets/SplitTabWidget.h"
#include "widgets/qmdiSplitTab.h"

SplitTabsPlugin::SplitTabsPlugin(TextEditorPlugin *p) : IPlugin() {
    name = tr("Split tabs support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    textEditorPlugin = p;
}

SplitTabsPlugin::~SplitTabsPlugin() {}

void SplitTabsPlugin::loadConfig(QSettings &settings) {}

void SplitTabsPlugin::saveConfig(QSettings &settings) {}

void SplitTabsPlugin::on_client_merged(qmdiHost *host) {
    IPlugin::on_client_merged(host);
    split = new qmdiSplitTab();
    split->setButtonProvider(new DefaultButtonsProvider);

    auto manager = getManager();
    auto splitAction = new QAction("Split tabs", split);
    splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));
    connect(splitAction, &QAction::triggered, splitAction, [this]() {
        split->splitHorizontally();
        textEditorPlugin->fileNew();
    });
    auto moveSplitAction = new QAction("Move editor to new split", split);
    moveSplitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backslash));
    connect(moveSplitAction, &QAction::triggered, moveSplitAction, [this]() {
        // TODO - this action should be disabled. I need expose the event of tabs modifies
        //        to 3rd parties, and not keep it an internal event.
        if (split->getWigetsCountInCurrentSplit() < 2) {
            return;
        }
        auto w = split->getCurrentWidget();
        split->moveTabToNewSplit(w);
    });
    connect(split, &qmdiSplitTab::newClientAdded, manager, &PluginManager::newClientAdded);

    manager->removeBuiltinActions();
    manager->menus["Se&ttings"]->addAction(splitAction);
    manager->menus["Se&ttings"]->addAction(moveSplitAction);
    manager->replaceMdiServer(split);
    manager->addBuiltinActions();
    manager->updateGUI();

    connect(manager, &PluginManager::newFileRequested, [this](QObject *s) {
        auto manager = getManager();
        auto tab = qobject_cast<QTabWidget *>(s->parent());
        auto index = split->findSplitIndex(tab);
        if (!tab || index < 0) {
            // textEditorPlugin->fileNew();
            return;
        }

        // we know where exactly to put this
        auto client = textEditorPlugin->fileNewEditor();
        auto editor = dynamic_cast<QWidget *>(client);
        split->addTabToSplit(index, editor, client->mdiClientName, client->mdiClientFileName());
        client->mdiServer = manager->getMdiServer();
        editor->setFocus();
    });
}

void SplitTabsPlugin::on_client_unmerged(qmdiHost *host) {}
