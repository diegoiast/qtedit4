#include "SplitViewPlugin.hpp"

#include "pluginmanager.h"
#include "widgets/qmdiSplitTab.h"

// first inter plugin dependency.
#include "plugins/texteditor/texteditor_plg.h"

SplitViewPlugin::SplitViewPlugin(TextEditorPlugin *editorPlugin) {
    name = tr("Split tabs view plugin");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    this->editorPlugin = editorPlugin;
}

void SplitViewPlugin::on_client_merged(qmdiHost *host) {
    auto pluginManager = static_cast<PluginManager *>(host);
    auto split = new qmdiSplitTab;
    auto splitAction = new QAction("Split tabs", split);
    split->setButtonProvider(new DefaultButtonsProvider);
    splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));

    connect(splitAction, &QAction::triggered, splitAction, [split, this]() {
        split->splitHorizontally();
        editorPlugin->fileNew();
    });

    auto moveSplitAction = new QAction("Move editor to new split", split);
    moveSplitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backslash));
    connect(moveSplitAction, &QAction::triggered, moveSplitAction, [split]() {
        // TODO - this action should be disabled. I need expose the event of tabs modifies
        //        to 3rd parties, and not keep it an internal event.
        if (split->getWigetsCountInCurrentSplit() < 2) {
            return;
        }
        auto w = split->getCurrentWidget();
        split->moveTabToNewSplit(w);
    });
    connect(split, &qmdiSplitTab::newClientAdded, pluginManager, &PluginManager::newClientAdded);

    pluginManager->removeBuiltinActions();
    pluginManager->menus["Se&ttings"]->addAction(splitAction);
    pluginManager->menus["Se&ttings"]->addAction(moveSplitAction);
    pluginManager->replaceMdiServer(split);
    pluginManager->addBuiltinActions();
    pluginManager->updateGUI();

    pluginManager->connect(
        pluginManager, &PluginManager::newFileRequested, [pluginManager, split, this](QObject *s) {
            auto tab = qobject_cast<QTabWidget *>(s->parent());
            auto index = split->findSplitIndex(tab);
            if (!tab || index < 0) {
                editorPlugin->fileNew();
                return;
            }

            // we know where exactly to put it
            auto client = editorPlugin->fileNewEditor();
            auto editor = dynamic_cast<QWidget *>(client);
            split->addTabToSplit(index, editor, client->mdiClientName, client->mdiClientFileName());
            client->mdiServer = pluginManager->getMdiServer();
            editor->setFocus();
        });
}

void SplitViewPlugin::loadConfig(QSettings &settings) {}

void SplitViewPlugin::saveConfig(QSettings &settings) {}
