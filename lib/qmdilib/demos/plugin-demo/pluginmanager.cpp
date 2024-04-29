/**
 * \file pluginmanager.cpp
 * \brief Implementation of the PluginManager class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL 2 or 3
 * \see PluginManager
 */

// $Id$

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolButton>

#include "configdialog.h"
#include "iplugin.h"
#include "pluginmanager.h"
#include "qmdihost.h"
#include "qmdiserver.h"
#include "qmditabwidget.h"

/**
 * \class PluginManager
 * \brief A class which manages a list of plugins and merges their menus and
 * toolbars to the main application \author Diego Iastrubni
 * (diegoiast@gmail.com)
 *
 * The plugin manager is a main window, and can save and restore it's state if a
 * settings manager has been defined. A settings manager (and instace of
 * QSettings) can be set to work on a local file (an ini file).
 *
 * The plugin manager is a mdi host (qmdiHost) which can load menus and toolbars
 * from a the selected mdi client mdi server, and also maintain another set of
 * mdi clients, which have no real GUI, but their menus and toolbars are merged
 * to the main host. This can be used to enable or disable functionality of the
 * application on run time - by enabling or disabling IPlugin objetcs.
 *
 * Each plugin has defines a set of menus and toolbars, and has some methods
 * for saving and restoring it's state. The menus and toolbars will create
 * actions visible on the main GUI which will trigger actions on the plugin.
 * Then the plugin can ask the mdi host to add a new mdi client.
 *
 * Plugins have also the concept of "new files", a set of commands which create
 * new files and an mdi client. Plugins also define the set of files which can
 * be opened, and can respond to requests to open a specific file.
 *
 * The plugin manager can also open a dialog to configure all the plugins
 * available, and enable or disable several plugins.
 *
 * A typical usage of this class will be:
 *
 * \code
 * QApplication app( argc, argv );
 * PluginManager pluginManager;
 * pluginManager.setFileSettingsManager( "settings.ini" );
 *
 * // load a few plugins
 * pluginManager.addPlugin( new ... );
 * pluginManager.addPlugin( new ... );
 * pluginManager.updateGUI();
 *
 * // start the application
 * pluginManager.restoreSettings();
 * return app.exec();
 * \endcode
 */

/**
 * \var PluginManager::plugins
 * \brief the list of plugins
 *
 * This is the list of plugins available on the system. It contains all plugins,
 * even ones disabled. When a plugin is added, it's appended to this list.
 */

/**
 * \var PluginManager::tabWidget
 * \brief the main widget of the form
 *
 * The plugin manager is built arround a QMainWindow, which it's main widget is
 * this tab widget. This serves also as the qmdiServer.
 *
 * \see qmdiServer
 */

/**
 * \var PluginManager::newFilePopup
 * \brief the "File/New" submenu
 *
 * This is the popup menu seen by the user when on the \b File menu.
 *
 * \see newFileActions()
 */

/**
 * \var PluginManager::actionOpen
 * \brief the open action
 *
 * This action is the one added to the \b File menu, as the \b Open... command.
 *
 * \see on_actionOpen_triggered()
 */

/**
 * \var PluginManager::actionClose
 * \brief the close action
 *
 * This action is the one added to the \b File menu, as the \b Close command.
 *
 * \see on_actionClose_triggered()
 */

/**
 * \var PluginManager::actionQuit
 * \brief the quit action
 *
 * This action is the one added to the \b File meun, as the quit command.
 *
 * \see on_actionQuit_triggered()
 */

/**
 * \var PluginManager::actionConfig
 * \brief the configuration action
 *
 * This action is the one added to the \b Settings menu, as the \b Configure
 * command.
 *
 * \see on_actionConfigure_triggered()
 */

/**
 * \var PluginManager::actionNextTab
 * \brief select the next tab action
 *
 * This action is the one added to the \b Settings menu, as the \b Next \b Tab
 * command.
 *
 * \see on_actionNext_triggered()
 */

/**
 * \var PluginManager::actionPrevTab
 * \brief select the previous tab action
 *
 * This action is the one added to the \b Settings menu, as the \b Previous
 * \b Tab command.
 *
 * \see on_actionPrev_triggered()
 */

/**
 * \var PluginManager::configDialog
 * \brief the configuration dialog
 *
 * An instace to the configuration dialog.
 */

/**
 * \var PluginManager::settingsManager
 * \brief the settings manager
 *
 * A simple pointer to a QSettings variable.
 */

/**
 * \brief default constructor
 *
 * Builds a plugin manager. Creates the actions needed for the "File" menu,
 * the "New" sub-menu, action for loading files, moving tabs etc.
 *
 * Sets the settingsManager to NULL, which means by default there is no option
 * to restore the state of the application.
 *
 * The config dialog is set to NULL - and will be created on demand.
 *
 * Eventually will call initGUI() to create the main GUI.
 *
 * \see initGUI()
 */
PluginManager::PluginManager() {
    configDialog = NULL;
    settingsManager = NULL;

    newFilePopup = new QMenu(tr("New..."), this);
    actionOpen = new QAction(tr("Open..."), this);
    actionClose = new QAction(tr("Close"), this);
    actionQuit = new QAction(tr("Ex&it"), this);
    actionConfig = new QAction(tr("&Config"), this);
    actionNextTab = new QAction(tr("&Next tab"), this);
    actionPrevTab = new QAction(tr("&Previous tab"), this);
    actionHideGUI = new QAction(tr("&Hide menus"), this);

    actionNextTab->setEnabled(false);
    actionPrevTab->setEnabled(false);
    actionClose->setEnabled(false);

    newFilePopup->setObjectName("newFilePopup");
    actionOpen->setObjectName("actionOpen");
    actionClose->setObjectName("actionClose");
    actionQuit->setObjectName("actionQuit");
    actionConfig->setObjectName("actionConfigure");
    actionNextTab->setObjectName("actionNext");
    actionPrevTab->setObjectName("actionPrev");
    actionHideGUI->setObjectName("actionHideGUI");
    actionHideGUI->setCheckable(true);

    newFilePopup->setIcon(QIcon::fromTheme("document-new"));
    actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    actionNextTab->setIcon(QIcon::fromTheme("go-next"));
    actionPrevTab->setIcon(QIcon::fromTheme("go-previous"));

    //	actionOpen->setIcon(
    // QIcon(":/trolltech/styles/commonstyle/images/diropen-32.png") );
    actionOpen->setIcon(QIcon::fromTheme("document-open"));
    actionOpen->setShortcut(QKeySequence("Ctrl+O"));

    //	actionClose->setIcon(
    // QIcon(":/trolltech/styles/commonstyle/images/standardbutton-cancel-32.png")
    //);
    actionClose->setIcon(QIcon::fromTheme("window-close"));
    actionClose->setShortcut(QKeySequence("Ctrl+w"));

    actionNextTab->setShortcut(QKeySequence("Alt+Right"));
    actionPrevTab->setShortcut(QKeySequence("Alt+Left"));
    actionHideGUI->setShortcut(QKeySequence("Ctrl+M"));

    addAction(actionOpen);
    addAction(actionClose);
    addAction(actionQuit);
    addAction(actionConfig);
    addAction(actionNextTab);
    addAction(actionPrevTab);
    addAction(actionHideGUI);

    metaObject()->connectSlotsByName(this);
    initGUI();
}

/**
 * \brief default destructor
 *
 * Deletes the object. If a settings manager is allocated it will be deleted as
 * well.
 *
 * \see setFileSettingsManager()
 * \see setNativeSettingsManager()
 * \see QSettings
 */
PluginManager::~PluginManager() {
    if (settingsManager) {
        saveSettings();
        delete settingsManager;
    }

    foreach (IPlugin *p, plugins) {
        if (plugins.removeAll(p) == 1)
            delete p;
        else {
            qDebug("%s - %d: could not remove plugin from the plugin manager (%s)", __FILE__,
                   __LINE__, qPrintable(p->getName()));
            return;
        }
    }
}

/**
 * \brief return the index of the tab in which this file is loaded
 * \param fileName the fully qualified file name to search
 * \return -1 if not found, otherwise the tab number
 *
 * This function will query all widgets in the QTabWidget of the main window
 * to see if they implement the qmdiClient interface. If a tab does implement
 * it - it will try and see if the mdiClientFileName() is the same as one passed
 * to this method.
 *
 * The method returns -1 if no mdi client is found that has loaded that file.
 * This means that if you load that file and insert it into the tab widget
 * directly, and not deriving qmdiClient this method will not see you new
 * widget.
 *
 * If the file has been found the method returns a number corresponding to the
 * tab in which the file has been loaded.
 *
 * \see openFile()
 */
int PluginManager::tabForFileName(QString fileName) {
    if (fileName.isEmpty())
        return -1;

    for (int i = 0; i < tabWidget->count(); i++) {
        qmdiClient *c = dynamic_cast<qmdiClient *>(tabWidget->widget(i));
        if (!c)
            continue;

        if (c->mdiClientFileName() == fileName)
            return i;
    }
    return -1;
}

/**
 * \brief set a native settings manager to this plugin manager
 * \param organization the organization your application belongs to
 * \param application the name of your application
 *
 * This method installs a QSettings in native mode (for windows this means using
 * the registry, and on *nix this means using an INI file) to be used to save
 * and restore settings. Any settings manager available before will be deleted.
 *
 * The \b organization and \b application parameters are passed to the
 * constructor of QSettings.
 *
 * \see setFileSettingsManager()
 * \see QSettings
 */
void PluginManager::setNativeSettingsManager(const QString &organization,
                                             const QString &application) {
    if (settingsManager)
        delete settingsManager;
    settingsManager = new QSettings(organization, application);
}

/**
 * \brief set an ini based settings manager to this plugin manaher
 * \param fileName the file to store the settings
 *
 * This method installs a QSettings in IniFormat mode (which means on all
 * platforms an INI file will be used) to be used to save and restore settings.
 * Any settings manager available before will be deleted.
 *
 * The \b fileName is the file to use as the backend. For more information
 * read the manual of QSettings.
 *
 * \see setNativeSettingsManager()
 * \see QSettings
 */
void PluginManager::setFileSettingsManager(const QString &fileName) {
    if (settingsManager)
        delete settingsManager;
    settingsManager = new QSettings(fileName, QSettings::IniFormat);
}

/**
 * \brief restore the state of the application
 *
 * Calling this method will restore the status of the main window
 * (size, position etc), and all loaded documents.
 *
 * Before loading the documents the plugin manager will ask all loaded plugins
 * to restore their state by calling IPlugin::loadConfig(). At the end this
 * method will also call updateActionsStatus()
 *
 * This method does nothing if no setting manager has been defined.
 *
 * \note When restoring the loaded documents, it may be possible to load a
 * document using a different plugin, if a "more suitable plugin" is available
 * when restoring the application state.
 *
 * \see IPlugin::loadConfig()
 * \see IPlugin::canOpenFile()
 * \see saveSettings()
 * \see updateActionsStatus()
 */
void PluginManager::restoreSettings() {
    if (!settingsManager)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    // restore window location
    settingsManager->beginGroup("mainwindow");
#if 0
	if (settingsManager->contains("maximized"))
		if (settingsManager->value("maximized").toBool())
			showMaximized();
		else
		{
			if (settingsManager->contains("location"))
				move( settingsManager->value("location").toPoint() );
			if (settingsManager->contains("size"))
				resize( settingsManager->value("size").toSize() );
// 			showNormal();
		}
	show();
#else
    if (settingsManager->contains("state"))
        restoreState(settingsManager->value("state", saveState()).toByteArray());
    if (settingsManager->contains("geometry"))
        restoreGeometry(settingsManager->value("geometry", saveGeometry()).toByteArray());
    if (settingsManager->value("maximized", false).toBool())
        setWindowState(windowState() | Qt::WindowMaximized);
    if (settingsManager->contains("size"))
        resize(settingsManager->value("size", size()).toSize());
    if (settingsManager->contains("location"))
        move(settingsManager->value("location", pos()).toPoint());
    if (settingsManager->contains("hidegui"))
        actionHideGUI->setChecked(settingsManager->value("hidegui", false).toBool());
#endif
    settingsManager->endGroup();

    show();
    statusBar()->showMessage(tr("Loading files..."), 5000);
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

    // restore opened files
    settingsManager->beginGroup("files");
    foreach (QString s, settingsManager->childKeys()) {
        if (!s.startsWith("file"))
            continue;
        QString fileName = settingsManager->value(s).toString();
        statusBar()->showMessage(tr("Loading file %1").arg(fileName), 5000);
        QApplication::processEvents();
        openFile(fileName);
    }

    // re-select the current tab
    int current = settingsManager->value("current", -1).toInt();
    if (current != -1)
        tabWidget->setCurrentIndex(current);

    statusBar()->clearMessage();
    settingsManager->endGroup();

    foreach (auto plugin, plugins) {
        plugin->loadConfig(*settingsManager);
    }

    updateActionsStatus();
}

/**
 * \brief save the state of the application into the settings manager
 *
 * This method stores the state of the window (size, position, etc) to the
 * settings manager. It will save the list of qmdiClients available on the tab
 * widget. If in one of the tabs there is a non mdi client instead of the
 * filename, in that the configuration file will save "@".
 *
 * This method will also call each one of the plugins and ask them to store
 * their configuration. Each plugin will have it's own section, named with the
 * plugin's name.
 *
 * This method will also sync the settings file.
 *
 * \see restoreSettings()
 * \see IPlugin::saveConfig()
 * \see QSettings::sync()
 */
void PluginManager::saveSettings() {
    if (!settingsManager)
        return;

    // main window state
    settingsManager->beginGroup("mainwindow");
    settingsManager->setValue("size", size());
    settingsManager->setValue("location", pos());
    settingsManager->setValue("maximized", isMaximized());
    settingsManager->setValue("state", saveState());
    settingsManager->setValue("geometry", saveGeometry());
    settingsManager->setValue("hidegui", actionHideGUI->isChecked());
    settingsManager->endGroup();

    // store saved files
    settingsManager->remove("files"); // remove all old loaded files
    if (tabWidget->count() != 0) {
        QString s;
        qmdiClient *c = NULL;
        settingsManager->beginGroup("files");
        for (int i = 0; i < tabWidget->count(); i++) {
            c = dynamic_cast<qmdiClient *>(tabWidget->widget(i));
            if (c)
                s = c->mdiClientFileName();
            else
                s.clear();

            if (!s.isEmpty())
                settingsManager->setValue(QString("file%1").arg(i), s);
            else {
                settingsManager->setValue(QString("file%1").arg(i), "@");
            }
        }

        settingsManager->setValue("current", tabWidget->currentIndex());
    }
    settingsManager->endGroup();

    // let each ones of the plugins save it's state
    foreach (IPlugin *p, plugins) {
        p->saveConfig(*settingsManager);
    }

    settingsManager->sync();
}

/**
 * \brief update some actions
 *
 * Calling this method will update the next/prev/close commands of the main
 * window.
 */
void PluginManager::updateActionsStatus() {
    int widgetsCount = tabWidget->count();
    actionClose->setEnabled(widgetsCount != 0);
    actionNextTab->setEnabled(widgetsCount > 1);
    actionPrevTab->setEnabled(widgetsCount > 1);
}

/**
 * \brief open a file, using the most suitable plugin
 * \param fileName a file name, or some king of URL
 * \param x dummy variable - see documentation of IPlugin::openFile()
 * \param y dummy variable - see documentation of IPlugin::openFile()
 * \param z dummy variable - see documentation of IPlugin::openFile()
 * \return false if the loading failed from any reason
 *
 * The plugin manager will try to find a plugin which is most suitable for
 * loading this file, by calling IPlugin::canOpenFile(). The plugin which
 * returns the highest score will be selected to open this file.
 *
 * \note It's not necesary for \b fileName to be a real file. It is possible to
 * ask for "loading" some "file", which can have any meaning you define in the
 * plugin handling this kind of "document".
 *
 * The parameters \b x \b y \b z will be sent to IPlugin::openFile(). Follow
 * the documentation for more details.
 *
 * \see IPlugin::canOpenFile()
 * \see IPlugin::openFile()
 * \todo how does a developer know why the loading of a file failed?
 */
bool PluginManager::openFile(QString fileName, int x, int y, int z) {
    // see if it's already open
    int i = tabForFileName(fileName);
    if (i != -1) {
        tabWidget->setCurrentIndex(i);
        return true;
    }

    // ok, not opened. who can open this file...?
    IPlugin *bestPlugin = NULL;
    IPlugin *p;
    int highestScore = -1;

    // see which plugin is the most suited for openning this file
    foreach (p, plugins) {
        if (!p->enabled)
            continue;

        // is this plugin better then the selected?
        i = p->canOpenFile(fileName);

        if (i > highestScore) {
            bestPlugin = p;
            highestScore = i; // bestPlugin->canOpenFile(fileName);
        }
    }

    // ask best plugin to open the file
    if (bestPlugin) {
        bool fileOpened = bestPlugin->openFile(fileName, x, y, z);
        if (fileOpened)
            updateActionsStatus();
        return fileOpened;
    } else
        // no plugin can handle this file,
        // this should not happen, and usually means a bug
        return false;
}

/**
 * \brief open a list of files
 * \param fileNames a list of files to load
 * \return false if any one of the file loadings failed
 *
 * Each one of the files will be opened by the most suitable plugin in the
 * system. The files will be opened with the default parameters (x=-1, y=-1,
 * z=-1).
 *
 * \see openFile()
 * \todo how does a developer know why the loading of one of the files failed?
 */
bool PluginManager::openFiles(QStringList fileNames) {
    QString s;
    bool b = true;
    foreach (s, fileNames) {
        b = b && openFile(s);
        QApplication::processEvents();
    }

    return b;
}

/**
 * \brief add a new plugin to the plugin manager system
 * \param newplugin the plugin to add to the system
 *
 * This will add the plugin to the system. If a settings manager is available,
 * the new plugin will be asked to load it's configuration from the settings
 * manager.
 *
 * If the auto enabled flag of the new plugin is enabled the plugin will get
 * enabled.
 *
 * Plugins added to the plugin manager are deleted by the plugin manager when it
 * is deleted, or when the plugin is removed from the list of plugins.
 *
 * \see IPlugin::autoEnabled
 * \see IPlugin::setEnabled()
 * \see IPlugin::loadConfig()
 * \see IPlugin::newFileActions);
 */
void PluginManager::addPlugin(IPlugin *newplugin) {
    plugins << newplugin;

    if (!newplugin)
        return;

    newplugin->mdiServer = dynamic_cast<qmdiServer *>(tabWidget);
    if (newplugin->alwaysEnabled)
        newplugin->autoEnabled = true;

    if (newplugin->autoEnabled)
        newplugin->enabled = true;

    if (newplugin->enabled)
        enablePlugin(newplugin);

    if (settingsManager)
        newplugin->loadConfig(*settingsManager);
}

/**
 * \brief remove a plugin from the plugin manager
 * \param oldplugin the plugin to be removed from the system
 *
 * When you call this method, any plugin passed to it will be removed from the
 * plugin manager and deleted (freeing it's memory).
 *
 * \see addPlugin
 * \see disablePlugin
 */
void PluginManager::removePlugin(IPlugin *oldplugin) {
    if (!oldplugin)
        return;

    disablePlugin(oldplugin);

    if (plugins.removeAll(oldplugin) == 1)
        delete oldplugin;
    else {
        qDebug("%s - %d: could not remove plugin from the plugin manager (%s)", __FILE__, __LINE__,
               qPrintable(oldplugin->getName()));
        return;
    }
}

/**
 * \brief enable a plugin in the system
 * \param plugin the plugin to enable
 *
 * This method will enable a plugin and merge it's menus and actions to the main
 * gui, this will add all the add all the "new actions" to the "File/New" sub
 * menu.
 *
 * \note this method works only on plugins available in the system, please use
 * PluginManager::addPlugin() before enabling a plugin.
 *
 * \see addPlugin()
 * \see disablePlugin()
 * \see qmdiHost::mergeClient()
 */
void PluginManager::enablePlugin(IPlugin *plugin) {
    if (!plugin)
        return;

    if (!plugins.contains(plugin)) {
        qDebug("%s - %d: tried to enable a plugin which was not part of the plugin "
               "manager (%s)",
               __FILE__, __LINE__, qPrintable(plugin->getName()));
        return;
    }

    if (plugin->enabled) {
        plugin->setEnabled(true);
        mergeClient(plugin);
    }

    QAction *a;
    QActionGroup *ag;
    ag = plugin->newFileActions();
    if (!ag)
        return;

    foreach (a, ag->actions()) {
        newFilePopup->addAction(a);
    }
}

/**
 * \brief disable a plugin in the system
 * \param plugin the plugin to disable
 *
 * This method will diable a plugin and unmerge it's menus and actions to the
 * main gui, this will add all the add all the "new actions" to the "File/New"
 * sub menu.
 *
 * \note this method works only on plugins available in the system, please use
 * PluginManager::addPlugin() before enabling a plugin.
 *
 * \todo remove new actions
 *
 * \see addPlugin()
 * \see enablePlugin()
 * \see qmdiHost::unmergeClient()
 */
void PluginManager::disablePlugin(IPlugin *plugin) {
    if (!plugin)
        return;

    if (!plugins.contains(plugin)) {
        qDebug("%s - %d: tried to disable a plugin which was not part of the "
               "plugin manager (%s)",
               __FILE__, __LINE__, qPrintable(plugin->getName()));
        return;
    }

    if (plugin->enabled) {
        plugin->setEnabled(false);
        unmergeClient(plugin);
    }
}

/**
 * \brief initialize the mdi client GUI
 *
 * This method initializes the main window's/mdi host GUI by creating
 * pre-defined menus and the standard actions provided by this calss.
 *
 * The actions available are:
 *  - File / New (a pop up menu, see IPlugin::newFileActions )
 *  - File / Open (see on_actionOpen_triggered() )
 *  - File / <- this is the merge point
 *  - File / Quit
 *  - Settings / Configure (see on_actionConfigure_triggered() )
 *  - Settings / Next tab (see on_actionConfigure_triggered() )
 *  - Settings / Previous tab (see on_actionConfigure_triggered() )
 *
 * The menus generated are (in this order)
 *  - File
 *  - Edit
 *  - Search
 *  - View
 *  - Project
 *  - Build
 *  - Debug
 *  - Navigation
 *  - Settings
 *  - Window
 *  - Help
 *
 * Menus which do not contain any actions will not be displayed on screen. In
 * future versions of this library there will be an option to add and remove
 * menus more freely.
 *
 * \todo add methods for adding/removing menus in a more sane way
 */
void PluginManager::initGUI() {
    menus[tr("&File")]->addMenu(newFilePopup);
    menus[tr("&File")]->addAction(actionOpen);
    menus[tr("&File")]->addSeparator();
    menus[tr("&File")]->setMergePoint();
    menus[tr("&File")]->addAction(actionClose);
    menus[tr("&File")]->addSeparator();
    menus[tr("&File")]->addAction(actionQuit);
    menus[tr("&Edit")];
    menus[tr("&Search")];
    menus[tr("&View")];
    menus[tr("&Project")];
    menus[tr("&Build")];
    menus[tr("&Debug")];
    menus[tr("&Navigation")];
    menus[tr("&Tools")];
    menus[tr("Se&ttings")]->addAction(actionConfig);
    menus[tr("Se&ttings")]->addSeparator();
    menus[tr("Se&ttings")]->addAction(actionNextTab);
    menus[tr("Se&ttings")]->addAction(actionPrevTab);
    menus[tr("Se&ttings")]->addAction(actionHideGUI);
    menus[tr("&Window")];
    menus[tr("&Help")];

    toolbars[tr("main")]->addAction(actionOpen);
    toolbars[tr("main")]->addAction(actionConfig);

    tabWidget = new qmdiTabWidget(this);
    updateGUI();

    QToolButton *tabCloseBtn = new QToolButton(tabWidget);
    connect(tabCloseBtn, SIGNAL(clicked()), this, SLOT(closeClient()));
    tabCloseBtn->setAutoRaise(true);
    //	tabCloseBtn->setIcon(QIcon(":images/closetab.png"));
    tabCloseBtn->setIcon(QIcon::fromTheme("window-close"));
    tabWidget->setCornerWidget(tabCloseBtn, Qt::TopRightCorner);

    QToolButton *addNewMdiClient = new QToolButton(tabWidget);
    connect(addNewMdiClient, SIGNAL(clicked()), addNewMdiClient, SLOT(showMenu()));
    addNewMdiClient->setAutoRaise(true);
    //	addNewMdiClient->setIcon(QIcon(":images/closetab.png"));
    addNewMdiClient->setIcon(QIcon::fromTheme("document-new"));
    addNewMdiClient->setMenu(newFilePopup);
    tabWidget->setCornerWidget(addNewMdiClient, Qt::TopLeftCorner);

    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);
}

/**
 * \brief close the current in the main tab widget
 *
 * This method will close the current tab in the tab widget. If the current
 * widget implements qmdiClient it will call qmdiClient::closeClient() otherwise
 * it will just delete the widget by calling QObject::deleteLater()
 */
void PluginManager::closeClient() {
    qmdiClient *client = dynamic_cast<qmdiClient *>(tabWidget->currentWidget());
    if (client == NULL)
        tabWidget->currentWidget()->deleteLater();
    else
        client->closeClient();
}

/**
 * \brief show the open dialog and load files
 *
 * This slot is connected to the \b triggered signal of the actionOpen. It will
 * display an open files dialog, and let the user choose which files to open and
 * eventually call openFiles().
 *
 * The extensions for the file dialog are computer from the list of plugins
 * available and enabled in the system.
 *
 * This slot is auto connected.
 *
 * \see IPlugin::extensAvailable()
 * \see openFiles()
 */
void PluginManager::on_actionOpen_triggered() {
    static QString workingDir;
    QString extens, e, allExtens;
    QStringList extensAvailable;
    IPlugin *p;

    // get list of available extensions to open from each plugin
    foreach (p, plugins) {
        if (!p->enabled)
            continue;
        extensAvailable << p->myExtensions();
    }

    int j = extensAvailable.size();
    for (int i = 0; i < j; ++i) {
        QString s = extensAvailable.at(i);
        extens += s;
        if (i < j - 1)
            extens += ";;";

        QRegularExpression regexp("\\((.*)\\)");
        auto m = regexp.match(s);
        QString s1 = m.captured(1).simplified();
        if (!s1.isEmpty()) {
            s1.remove("*.*");
            s1.remove(QRegularExpression("\\b*\\b"));
            allExtens += " " + s1;
        }
    }
    // all supported files is the first item
    extens = tr("All supported files") + QString(" (%1);;").arg(allExtens) + extens;

    // 	TODO do we need to add "all files"?
    // 	extens = extens + ";;" + tr("All files") + " (*.*)";
    // 	qDebug("all extensions: %s", qPrintable(allExtens) );

    QStringList s = QFileDialog::getOpenFileNames(NULL, tr("Choose a file"), workingDir, extens);

    if (s.isEmpty())
        return;

    openFiles(s);
}

/**
 * \brief close the current widget
 *
 * This slot will close the current document, by calling
 * qmdiTabWidget::tryCloseClient()
 *
 * This slot is auto connected. This slot is triggered by the actionClose found
 * in the \b File menu.
 *
 * \see qmdiTabWidget::tryCloseClient()
 * \todo fix this to be calculated when tabs are open or closed, do this via a
 * signal from QTabWidget (qmdiServer?)
 */
void PluginManager::on_actionClose_triggered() {
    tabWidget->tryCloseClient(tabWidget->currentIndex());

    // TODO fix this to be calculated when tabs are open
    //      or closed, do this via a signal from QTabWidget (qmdiServer?)
    updateActionsStatus();
}

/**
 * \brief quit the application
 *
 * Quits the application.
 *
 * This slot is auto connected. This slot is triggered by the actionQuit
 * found in the \b File menu.
 */
void PluginManager::on_actionQuit_triggered() { this->close(); }

/**
 * \brief configure the available plugins
 *
 * This will bring up the plugin configuration dialog from which the user can
 * configure the available plugins.
 *
 * This slot is auto connected. This slot is triggered by the actionConfigure
 * found in the \b Settings menu.
 */
void PluginManager::on_actionConfigure_triggered() {
    if (!configDialog) {
        configDialog = new ConfigDialog(this);
        configDialog->setManager(this);
    }

    configDialog->show();
    configDialog->setFocus();
}

/**
 * \brief select the previous (left) tab
 *
 * This slot will select on the tab widget the previous (generally the left) tab
 * and activate it. This methods does not cycle trough (meaning, when you are on
 * the first tab, calling it will not move to the last tab)
 *
 * This slot is auto connected. This slot is triggered by the actionPrevTab
 * found in the \b Settings menu.
 *
 * \see on_actionNext_triggered()
 * \see PluginManager::actionPrevTab
 */
void PluginManager::on_actionPrev_triggered() {
    int i = tabWidget->currentIndex();
    if (i == 0)
        return;

    i--;
    tabWidget->setCurrentIndex(i);
}

/**
 * \brief select the next (right) tab
 *
 * This slot will select on the tab widget the next (generally the right) tab
 * and activate it. This methods does not cycle trough (meaning, when you are on
 * the laft tab, calling it will not move to the first tab)
 *
 * This slot is auto connected. This slot is triggered by the actionNextTab
 * found in the \b Settings menu.
 *
 * \see on_actionPrev_triggered()
 * \see PluginManager::actionNextTab
 */
void PluginManager::on_actionNext_triggered() {
    int i = tabWidget->currentIndex();
    if (i == tabWidget->count())
        return;

    i++;
    tabWidget->setCurrentIndex(i);
}

void PluginManager::on_actionHideGUI_changed() {
    qmdiClient *currentClient = dynamic_cast<qmdiClient *>(tabWidget->currentWidget());

    // if GUI disabled, unmerge the current item, else merge it
    // this way when enabling back you don't see some actions twice
    if (currentClient && actionHideGUI->isChecked()) {
        unmergeClient(currentClient);
        updateGUI();
    }

    updateMenusAndToolBars = !actionHideGUI->isChecked();
    setUpdatesEnabled(false);
    statusBar()->setVisible(!actionHideGUI->isChecked());
    menuBar()->setVisible(!actionHideGUI->isChecked());
    foreach (QToolBar *b, findChildren<QToolBar *>()) {
        b->setVisible(!actionHideGUI->isChecked());
    }

    foreach (QDockWidget *d, findChildren<QDockWidget *>()) {
        if (!actionHideGUI->isChecked())
            d->setFeatures(d->features() | QDockWidget::DockWidgetMovable |
                           QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
        else
            d->setFeatures(d->features() & ~QDockWidget::DockWidgetMovable &
                           ~QDockWidget::DockWidgetClosable & ~QDockWidget::DockWidgetFloatable);
    }

    if (currentClient && !actionHideGUI->isChecked()) {
        mergeClient(currentClient);
        updateGUI();
    }

    setUpdatesEnabled(true);
}
