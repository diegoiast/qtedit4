/**
 * \file qmdihost.cpp
 * \brief Implementation of the qmdi host class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiHost
 */

#include "qmdihost.h"
#include "qmdiclient.h"
#include <QAction>
#include <QMainWindow>

/**
 * \class qmdiHost
 * \brief The class which contain the menus and toolbars defined by qmdiClient
 *
 * Every time a user selects a new widget in the MDI server (for example
 * qmdiTabWidget), the server will try and ask the host to modify the menus. The
 * interface in which the QTabWidget and QMainWindow talk is this class.
 *
 * Generally speaking, you don't have to know much of this class, only inherit
 * it in your main windows.
 */

/**
 * \var qmdiHost::menus
 * \brief the default menus defined in this MDI host
 *
 * When you construct a window which derives this class,
 * you cannot define the menus the way which is dictated by
 * Qt. Instead you must follow the code guidelines provided by
 * this library.
 *
 * The menus defined in this class, will be displayed by default
 * on the MDI host. It is a wise idea to define the order of the
 * menus on your application on the initialization of the application,
 * since there is no way of changing the order of the menus later
 * on.
 *
 * \code
 * menus["&File"];
 * menus["&Edit"];
 * menus["&Help"];
 * \endcode
 *
 * \see \ref small_tutor
 * \see qmdiServer
 * \see qmdiActionGroupList
 */

/**
 * \var qmdiHost::toolbars
 * \brief the default toolbars defined in this MDI host
 *
 * When you construct a window which derives this class,
 * you cannot define the toolbars the way which is dictated by
 * Qt. Instead you must follow the code guidelines provided by
 * this library.
 *
 * The toolbars defined in this class, will be displayed by default
 * on the MDI host.
 *
 * \see \ref small_tutor
 * \see qmdiServer
 * \see qmdiActionGroupList
 */

/**
 * \var qmdiHost::toolBarList
 * \brief internal list of available toolbars
 *
 * As Qt4 does not provide an interface for listing the available
 * toolbars, the list is maintained as a separate list.
 *
 * You should usually not use this list directly.
 *
 * \internal
 * \todo how about quering the childs of the main window? it should be possible
 * to remove the toolBarList
 */

/**
 * \var qmdiHost::updateMenusAndToolBars
 * \brief lock modifications of menus and toolbars
 *
 * If this variable is set to true (default) when merging the menus/toolbars
 * of a qmdiClient the menus and toolbars of the host are modified as well.
 *
 * You can disable this if you don't want to update the menus or toolbars when
 * changing tabs in a qmdiTabWidget for example.
 *
 * \since 0.0.5
 * \see mergeClient
 */

/**
 * \brief default constructor
 *
 * Construct a qmdiHost instance.
 */
qmdiHost::qmdiHost() {
    updateMenusAndToolBars = true;
    toolBarList = nullptr;
}

/**
 * \brief Default destructor
 *
 * This destructor deletes \b toolBarList
 *
 * \see toolBarList
 */
qmdiHost::~qmdiHost() { delete toolBarList; }

/**
 * \brief update the toolbars and menus
 * \param window the window to update
 *
 * This function generates the menubar and toolbars from
 * the toolbar and menus definitions.
 *
 * You should call this method after every time you modify the menus
 * or structures.
 *
 * The parameter window should be generally \b this , it's passed on
 * as a parameter, since qmdiHost cannot dynamic_cast it self to an
 * QObject (this just does not work). On the other hand, this can give
 * you more freedom, as you do not have to derive the main window
 * also from qmdiHost, and the host can be a separate object.
 *
 * Since version 0.0.4, the \b window parameter is optional. This method
 * will try and see it \b this is a QMainWindow, and then update itself.
 *
 * \see qmdiActionGroupList
 */
void qmdiHost::updateGUI(QMainWindow *window) {
    // if passed nullptr, lets try another sanity check
    if (window == nullptr)
        window = dynamic_cast<QMainWindow *>(this);

    if (window == nullptr) {
        qDebug("%s - warning, no QMainWindow specified", __FUNCTION__);
        return;
    }

    if (!updateMenusAndToolBars)
        return;

    window->setUpdatesEnabled(false);
    toolBarList = toolbars.updateToolBar(toolBarList, window);
    menus.updateMenuBar(window->menuBar());
    window->setUpdatesEnabled(true);
}

/**
 * \brief merge the toolbars and menus of another MDI client
 * \param client the client to be merged
 *
 * This function is used to merge the toolbars and contents
 * of the MDI client to be merged into this client. The menus
 * and toolbars of the host will be appended to the end
 * of the menus and toolbars of this MDI host.
 *
 * This method will announce the client that it's been merged
 * by calling qmdiClient::on_client_merged()
 *
 * After a call to this function, you should manually call
 * updateGUI.
 *
 * Since version 0.0.5 this method also tries to cast the client to a widget,
 * and if it's succesful, it will add all it's menus+toolbars actions to the
 * widget. This way when changing tabs in a qmdiHost (like qmdiTabWidget) and
 * menus are hidden, the actions are still available.
 *
 * Since version 0.0.5 this method will update the menus and toolbars only if
 * updateMenusAndToolBars is true (default).
 *
 * \see updateGUI
 * \see unmergeClient
 * \see updateMenusAndToolBars
 * \see qmdiClient::on_client_merged()
 */
void qmdiHost::mergeClient(qmdiClient *client) {
    if (client == nullptr)
        return;

    if (updateMenusAndToolBars) {
        menus.mergeGroupList(&client->menus);
        toolbars.mergeGroupList(&client->toolbars);
    }
    client->on_client_merged(this);

    QWidget *w = dynamic_cast<QWidget *>(client);
    if (!w)
        return;
    addActionsToWidget(client->menus, w);
    addActionsToWidget(client->toolbars, w);
}

/**
 * \brief merge the toolbars and menus of another MDI client
 * \param client the client to be merged
 *
 * This function is used to un-merge the toolbars and contents
 * of the MDI client to be un-merged into this client. The menus
 * and toolbars of host will be updated, and all the entries
 * defined in the client will be removed.
 *
 * This method will announce the client that it's been unmerged
 * by calling qmdiClient::on_client_unmerged()
 *
 * After a call to this function, you should manually call
 * updateGUI.
 *
 * Since version 0.0.5 this method also tries to cast the client to a widget,
 * and if it's succesful, it will remove all it's menus+toolbars actions from
 * the widget.
 *
 * Since version 0.0.5 this method will update the menus and toolbars only if
 * updateMenusAndToolBars is true (default).
 *
 * \see updateGUI
 * \see mergeClient
 * \see updateMenusAndToolBars
 * \see qmdiClient::on_client_unmerged()
 */
void qmdiHost::unmergeClient(qmdiClient *client) {
    if (client == nullptr)
        return;

    if (updateMenusAndToolBars) {
        menus.unmergeGroupList(&client->menus);
        toolbars.unmergeGroupList(&client->toolbars);
    }
    client->on_client_unmerged(this);

    QWidget *w = dynamic_cast<QWidget *>(client);
    if (!w)
        return;
    removeActionsFromWidget(client->menus, w);
    removeActionsFromWidget(client->toolbars, w);
}

/**
 * \brief add a list of actions to a widget
 * \param agl the action group list to look for actions in
 * \param w the target widget
 *
 * This method adds all the actions in the action group list provided, to the
 * widget passed on.
 *
 * It's used internally by mergeClient()
 *
 * \see mergeClient
 * \see QWidget::addAction
 */
void qmdiHost::addActionsToWidget(qmdiActionGroupList &agl, QWidget *w) {
    foreach (qmdiActionGroup *g, agl.actionGroups)
        foreach (QObject *o, g->actionGroupItems) {
            QAction *a = qobject_cast<QAction *>(o);
            if (!a)
                continue;

            if (w->actions().contains(a))
                continue;
            w->addAction(a);
        }
}

/**
 * \brief remove a list of actions from a widget
 * \param agl the action group list to look for actions in
 * \param w the target widget
 *
 * This method removes all the actions in the action group list provided, to the
 * widget passed on.
 *
 * It's used internally by unmergeClient()
 *
 * \see unmergeClient
 * \see QWidget::addAction
 */
void qmdiHost::removeActionsFromWidget(qmdiActionGroupList &agl, QWidget *w) {
    foreach (qmdiActionGroup *g, agl.actionGroups)
        foreach (QObject *o, g->actionGroupItems) {
            QAction *a = qobject_cast<QAction *>(o);
            if (!a)
                continue;

            if (!w->actions().contains(a))
                continue;
            w->removeAction(a);
        }
}
