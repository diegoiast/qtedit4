/**
 * \file iplugin.cpp
 * \brief Implementation of the IPlugin interface
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL 2 or 3
 * \see IPlugin
 */

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QSettings>

#include "iplugin.h"

/**
 * \class IPlugin
 * \brief an abstract interface for defining Plugins
 *
 * This class is the base class for defining plugins using qmdilib. Plugins are
 * nothing more then a more specialized qmdiClient, with much more extra
 * methods.
 *
 * Plugins are are merged to the main window, generally a PluginManager class.
 * A plugin can define a list of extensions it can handle using canOpenFile(),
 * files it can load using the open dialog, and which files it can create.
 *
 * The different between canOpenFile() and the list of extensions it can open
 * is demostrated by the help plugin demo: the plugin can open files with this
 * kind of "url": \b help:QString , \b
 * help:///usr/share/doc/qt4/doc/html/opensourceedition\b .html while it cannot
 * directly open "help files" from the open file dialog.
 *
 * The PluginManager::openFile() method can be used for communications between
 * plugins. Again using the help plugin demo: a text editor can ask to help a
 * help by asking the plugin manager to load a "help:qstring" file. The plugin
 * manager will query all the available plugins and if someone can handle such
 * file it will ask the most suitable plugin to load the file.
 *
 * \code
 * PluginManager *m = dynamic_cast<PluginManager*>(mdiServer->mdiHost);
 * QMainWindow   *w = qobject_cast<QMainWindow*>(this->window());
 * QTextCursor cursor = textCursor();
 * cursor.select(QTextCursor::WordUnderCursor);
 * if (!m->openFile( "help:" + cursor.selectedText().toLower() ))
 * {
 * 	// lets inform the user
 * 	if (w)
 * 		w->statusBar()->showMessage( "Could not find help for this
 * keyword", 5 * 1000 ); // this is msec
 * }
 * \endcode
 *
 * The plugin manager also has a GUI for configuring each plugin. The assumption
 * is that every client added to the PluginManager will get notified about the
 * configuration change and will react. This policy is not enforced and you as
 * the author of the plugin is supposed to make this happen.
 *
 * The plugin manager can also disable or enable a plugin by calling
 * setEnabled() unless the plugin is marked as IPlugin::alwaysEnabled
 *
 * Note: when defining a new plugin, it must be declared
 * in this form, otherwise the application will crahs in
 * random and funny places:
 *
 * \code
 * class MyPlugin: public IPlugin
 * {
 * 	Q_OBJECT
 * };
 * \endcode
 *
 * \see PluginManager
 * \see qmdiClient
 */

/**
 * \var IPlugin::name
 * \brief the name of this plugin
 *
 * This string contains the name of the plugin. It should be
 * a small text describing it's functionality.
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/**
 * \var IPlugin::author
 * \brief the author of this plugin
 *
 * This string contains the name of the author (and email).
 *
 * This field is displayed in the plugin manager's dialog and does not define
 * any special functionality.
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/** \var IPlugin::sVersion
 * \brief the string version of this plugin
 *
 * This string contains the version of this plugin, for example "4.0.1". This
 * should match \b qmdiHost::iVersion
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/** \var IPlugin::iVersion
 * \brief the numeral version of this plugin
 *
 * This number should define the verion of this plugin. It's up to you to define
 * how to construst a version number, but you should be careful to increase its
 * value as needed.
 *
 * One way to encode the version supplied by qmdiHost::sVersion is
 * sVersion = "4.2.7" -> iVersion = 4*100 + 2*10 + 7*1 = 427 .
 *
 * There is no official policy that dictates how this should be done and you
 * can use any other versioning scheme (even numbering the releases normally
 * like: version 1,2...).
 *
 * This filed is not used at the current implementation, but in future releases
 * it may be used to call upgrade methods from an older version of the plugin to
 * a new one. This can be used to perform any upgrades to configuration or
 * data-bases used by this plugin.
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/**
 * \var IPlugin::alwaysEnabled
 * \brief defines if the plugin can be disabled
 *
 * If this flag is set to true, any call to setEnabled(false) should have no
 * effect. Also, the GUI of the plugin manager will not let you disable the
 * plugin.
 *
 * By default this is set to \b false, which means the plugin can be disabled.
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/**
 * \var IPlugin::autoEnabled
 * \brief defines the state of the plugin once the application starts
 *
 * This flag tells the plugin manager the state of this plugin once the
 * application starts. When the application will start the plugin manager will
 * enable the plugin if needed.
 *
 * By default this is set to \b false , which means that the plugin will not
 * be enabled when the application starts, and the user needs to enable it or
 * the main application can enable it programatically by calling
 * setEnabled(true)
 *
 * Please change this value \b only when constructing the class, and not during
 * it's life time. You may consider this a read only variable outside the
 * constructor.
 */

/**
 * \var IPlugin::enabled
 * \brief describes the status of this plugin
 *
 * The enabled flag is set to true when the plugin is enabled, and it's
 * menus and toolbars are merged into the plugin manager's main window.
 *
 * This flag should not be modified directly - instead of setEnabled().
 *
 * \internal
 * \see setEnabled
 */

/**
 * \brief ugly helper function to create a new action
 * \param icon the icon to assign to this new action
 * \param name the name to assign to this new action
 * \param obj the to connect to
 * \param shortcut the shortcut to assign to this new action
 * \param status_tip the status tip to assign to this new action
 * \param slot the slot to connect to
 * \return a new QAction object construsted from the parameters passed
 *
 * Create a new QAction and assign to it a name, icon status bar tip and
 * shortcut. Then connect the \b triggered() signal to the slot passed as a
 * parameter.
 */
QAction *new_action(QIcon icon, QString name, QObject *obj, QString shortcut, QString status_tip,
                    const char *slot) {
    QAction *a = new QAction(icon, name, obj);
    a->setShortcut(shortcut);
    a->setStatusTip(status_tip);
    a->connect(a, SIGNAL(triggered()), obj, slot);

    return a;
}

/**
 * \brief default constructor for IPlugin
 *
 * Construct an IPlugin interface, and set the default values.
 * In derived constructors you should set the name, author , iVersion and
 * sVersion fileds - otherwise the behaviour of this plugin is not defined.
 */
IPlugin::IPlugin() {
    alwaysEnabled = false;
    autoEnabled = false;
    enabled = false;
}

/**
 * \brief default desctructor
 *
 * Destruct the class, does nothing special.
 */
IPlugin::~IPlugin() {}

/**
 * \brief show the about dialog of this plugin
 *
 * The plugin manager configuration dialog calls this method when the user wants
 * to see the about dialog of this plugin.
 *
 * The method should display a modal window, with information about the plugin
 * and the author. Using QMessageBox is ideal, as it's code is very minimal
 * and easy to use.
 *
 * There is no need to implement this method in derived classes - if not derived
 * when the user pressed "About" in the configuration dialog nothing will
 * happen.
 *
 * The default implementation does nothing.
 */
void IPlugin::showAbout() {}

/**
 * \brief get the configuration dialog of this plugin
 *
 *
 */
QWidget *IPlugin::getConfigDialog() { return NULL; }

void IPlugin::getData() {}

void IPlugin::setData() {}

/**
 * \brief restore the state of the plugin from the settings manager
 * \param settings the QSettings to load configuration from
 *
 * This method is called by the PluginManager::restoreSettings().
 *
 * Derived plugins need to load any settings needed from the QSettings instance
 * passed.
 *
 * The default implementation does nothing.
 *
 * \see IPlugin::saveConfig()
 */
void IPlugin::loadConfig(QSettings &settings) { Q_UNUSED(settings); }

/**
 * \brief save the state of the plugin to the settings manager
 * \param settings the QSettings to save configuration to
 *
 * This method is called by the PluginManager::saveSettings().
 *
 * Derived plugins need to save any settings needed from to QSettings instance
 * passed.
 *
 * The default implementation does nothing.
 *
 * \see IPlugin::loadConfig()
 */
void IPlugin::saveConfig(QSettings &settings) { Q_UNUSED(settings); }

/**
 * \brief define the "new actions" supported by this plugin
 * \return the list of new actions this plugin defines
 *
 * The plugin manager will ask this plugin which kind of "new files" it can
 * generate. The returned list should be a QActionGroup containing QActions.
 * Each QAction should be connected to a slot in the derived class to create
 * a "new file". By default returns NULL.
 *
 * The actions and the action group will be deleted only by the desctructor of
 * your new plugin. This can be done by creating the action group and parenting
 * it to the plugin.
 *
 * The "new actions" will be displayed in the "File" menu of the plugin
 *manager's main window, as a sub menu, File->New-> (your actions). In this sub
 *menu the user will see all the "new actions" of all the available plugins. The
 *order of appearence in that sub menu is not deterministic - the first which
 *gets loaded is first to be displayed (this may change in future releases).
 *
 * Example
 *\code
 * newplugin::newplugin()
 * {
 *	// more code...
 * 	actionNew	= new QAction( this, tr("Create a new file") );
 * 	connect( actionNew, SIGNAL(triggered()), this, SLOT(fileNew()));
 *
 * 	_newFileActions = new QActionGroup(this);
 * 	_newFileActions->addAction( actionNew );
 *	// more code...
 * }
 *
 * QActionGroup* newplugin::newFileActions()
 * {
 * 	return _newFileActions;
 * }
 *
 * void newplugin::fileNew()
 * {
 *	// this example directly adds a new qmdiClient
 *	// but you may pop up a wizard menu to get feedback from the user and
 *	// then do something else.
 *
 * 	if (!mdiServer)
 * 	{
 * 		qDebug("%s - %d : warning no mdiServer defined", __FUNCTION__,
 *__LINE__ ); return;
 * 	}
 *
 * 	QexTextEdit *editor = new QexTextEdit(QString(), true);
 * 	editor->mdiClientName = tr("No name");
 * 	editor->setObjectName( editor->mdiClientName );
 * 	mdiServer->addClient( editor );
 *  }
 * \endcode
 *
 * \see myExtensions()
 */
QActionGroup *IPlugin::newFileActions() {
    // by default plugins cannot create any files
    return NULL;
}

/**
 * \brief define the list of extensions this plugin can load
 * \return a QStringList which contains the extensions this plugin wants to
 *
 * When the plugin manager's main window displays the open file dialog, it will
 * ask all the loaded and enabled plugins what extensions are to display. The
 * plugin manager will combine all the extensions available from all enabled
 * modules into a single list and then will use the list to display the open
 * file dialog.
 *
 * The order of appearence in the open file dialog is non deterministic - the
 * first plugin which gets loaded is first to be displayed (this may change in
 * future releases).
 *
 * example:
 * \code
 * QStringList newplugin::myExtensions()
 * {
 * 	QStringList s;
 * 	s << tr("Sources"	, "newplugin::myExtensions")	+ " (*.c *.cpp
 * *.cxx *.h *.hpp *.hxx *.inc)"; s << tr("Headers"	,
 * "newplugin::myExtensions")	+ " (*.h *.hpp *.hxx *.inc)"; s << tr("Qt
 * project"	, "newplugin::myExtensions")	+ " (*.pro *.pri)"; s << tr("All
 * files"	, "newplugin::myExtensions")	+ " (*.*)"; return s;
 * }
 * \endcode
 *
 * \b Note If putting an extension in this list does not garantee that the
 * plugin manager will choose such file to be opened with this plugin. This mean
 * that if you asked for extention \b *.gif but there is another plugin which
 * can load a *.gif with an higher scrore - the plugin manager will choose the
 * other plugin, even tough this plugin was the "cause" for loading the file.
 *
 * The default implementation returns an empty string list.
 *
 * \see newFileActions() , canOpenFile()
 */
QStringList IPlugin::myExtensions() {
    // no available extensions by default
    return QStringList();
}

/**
 * \brief return a score for loading a file
 * \param fileName the file to load
 * \return the score this plugin gives to a file passed as a parameter
 *
 * When the plugin manager tries to load a file, it will query all available and
 * enabled plugins for their score for loading that file. Evenutually the plugin
 * manager will choose the most suitable plugin for loading that specific file.
 *
 * By default this returns -1, which means you \b must change the returned value
 * on derived plugins - otherwise it's proven that your plugin will not be
 * selected.
 *
 * On large projects, with many plugins - it's wise to keep a table of all
 * plugins available and the files they can load. Otherwise you will have
 * difficulties tracking all the plugins available.
 *
 * The fileName is not garanteed to be a real file name. It could be a URL or a
 * wild card. On some applications, you may assume that is the fileName has no
 * scheme - it's a file (as demostrated in one of the demos).
 */
int IPlugin::canOpenFile(const QString fileName) {
    // can't open no file
    return -1;
    Q_UNUSED(fileName);
}

/**
 * \brief open a file
 * \param fileName the file to open
 * \param x dummy variable - see documentation
 * \param y dummy variable - see documentation
 * \param z dummy variable - see documentation
 * \return true if the file has been loaded
 *
 * This method is usually called by the plugin manager, once it calculated that
 * this plugin has the higher score for loading this file.
 *
 * This method should create a new mdi client which is capable of loading the
 * file and insert it into the mdi server of this plugin.
 *
 * The parameters sent to this function should mean something to this plugin,
 * but there is no real need to use them. For example, a plugin which displays
 * a 2D view of the world (like google maps or something similar), (x,y) can be
 * the coordinates and (z) the elevation. A text editor might use (x,y) and
 * ignore all (z) sent to it, etc.
 *
 * You need to remember that this can be also used to communicate between
 * plugins. One of the examples supplied in this library creates a help plugin
 * which can load "help:/" files, and a text editor which asks to load files,
 * the help plugin says "yes", and then the openFile() method of the help plugin
 * selects the mdi client which contains the help and then changes its URL.
 *
 * \todo when the file loading fails, how can the programmer know why it failed?
 *
 * \see IPlugin::newFileActions()
 */
bool IPlugin::openFile(QString fileName, int x, int y, int z) {
    // refuse to open any file
    return false;
    Q_UNUSED(fileName);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(z);
}

void IPlugin::navigateFile(qmdiClient *client, int x, int y, int z) {
    Q_UNUSED(client);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(z);
}

/**
 * \brief return the status of this plugin
 * \return the value of enabled
 *
 * Use this method for quering the plugin interface, and do not use
 * IPlugin::enabled directly.
 *
 * \see IPlugin::enabled
 */
bool IPlugin::isEnabled() { return enabled; }

/**
 * \brief set the status of this plugin
 * \param enable the new state of this plugin
 *
 * Use this method for setting the status of the plugin interface, and do not
 * use IPlugin::enabled directly.
 *
 * \see IPlugin::enabled
 */
void IPlugin::setEnabled(bool enable) { enabled = enable; }

/**
 * \brief returns if this plugin can be disabled
 * \return the opposite of the internal variable alwaysEnabled
 *
 * There are some plugins which cannot be disabled, and they are always enabled.
 *
 * \see IPlugin::alwaysEnabled
 */
bool IPlugin::canDisable() { return !alwaysEnabled; }

/**
 * \brief the icon displayed in the plugin manager configuration dialog
 *
 * This is the icon representing the plugin the plugin manager configuration
 * dialog.
 *
 * \todo what is the size of the icon?
 * \todo what about caching the icon?
 */
QIcon IPlugin::getIcon() { return QIcon("images/config.png"); }

/**
 * \brief return the name of this plugin
 * \return the name of this plugin
 *
 * Use this method for quering the plugin interface, and do not use
 * IPlugin::name directly.
 *
 * \see IPlugin::name
 */
QString IPlugin::getName() { return name; }

/**
 * \brief return the author of this plugin
 * \return the author name of this plugin
 *
 * Use this method for quering the plugin interface, and do not use
 * IPlugin::author directly.
 *
 * \see IPlugin::author
 */
QString IPlugin::getAuthor() { return author; }

/**
 * \brief return the string version of this plugin
 * \return the string version of this plugin
 *
 * Use this method for quering the plugin interface, and do not use
 * IPlugin::sVersion directly.
 *
 * \see IPlugin::sVersion
 */
QString IPlugin::getsVersion() { return sVersion; }

/**
 * \brief return the integer version of this plugin
 * \return the internal version of this plugin
 *
 * Use this method for quering the plugin interface, and do not use
 * IPlugin::iVersion directly.
 *
 * \see IPlugin::iVersion
 */
int IPlugin::getiVersion() { return iVersion; }
