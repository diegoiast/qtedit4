/**
 * \file qmdiactiongrouplist.cpp
 * \brief Implementation of the action group list class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiActionGroupList
 */

#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include <QString>
#include <QToolBar>

#include "qmdiactiongroup.h"
#include "qmdiactiongrouplist.h"

/**
 * \class qmdiActionGroupList
 * \brief abstraction layer for QMenuBar and a list of toolbars
 *
 * This class defines a QMenuBar and the list of toolbars available on
 * a typical application. Each sub-menu or toolbar is defined by one
 * qmdiActionGroup.
 *
 * This class has also the ability to merge other qmdiActionGroupList (this
 * enables widgets to add their partial menus to the menus supplied by the main
 * application).
 */

/**
 * \var qmdiActionGroupList::actionGroups
 * \brief list of merged in groups
 *
 * This is the list of merged in sub-groups.
 *
 * \internal
 * \see mergeGroup()
 */

/**
 * \brief default constructor
 *
 * Build an empty action group list. If you generate a menubar
 * from this empty class, you will get a NIL menu. Generating
 * a toolbar set from this empty class will generate no toolbars.
 */
qmdiActionGroupList::qmdiActionGroupList() {}

qmdiActionGroupList::~qmdiActionGroupList() { actionGroups.clear(); }

/**
 * \brief overloaded operator for getting the instance of a action group
 * \param name the action group name you want to get
 * \return an instance of an action group
 *
 * This is just an overloaded function which calls getActionGroup().
 *
 * \todo why not "const QSting&" ?
 * \see getActionGroup()
 */
qmdiActionGroup *qmdiActionGroupList::operator[](const QString name) {
    return getActionGroup(name);
}

/**
 * \brief get the instance of a action group
 * \param name the action group name you want to get
 * \return an instance of an action group
 *
 * This function returns an instance of a action group. Action groups
 * are abstractions of QMenu and QToolBar.
 *
 * If the action group requested is not available, a new instance will be
 * created.
 *
 * \see updateMenu()
 * \see updateToolBar()
 */
qmdiActionGroup *qmdiActionGroupList::getActionGroup(const QString &name) {
    qmdiActionGroup *item = nullptr;

    foreach (qmdiActionGroup *i, actionGroups) {
        if (i->getName() == name)
            return i;
    }

    // if menu does not exist, create it
    item = new qmdiActionGroup(name);
    actionGroups.append(item);
    return item;
}

/**
 * \brief merge another action group list
 * \param group the new group to merge into this one
 *
 * This function merges an action group list definition into this
 * action group list:
 *  - If in the new group there are action groups, the items will be appended to
 * the existing ones
 *  - If in the new group there are new actions groups, those groups will be
 * added to this action group list
 *
 * Note that just merging is not enough, and you might need also to update
 * the real widget which this action group list represents.
 *
 * \see unmergeGroupList
 * \see updateMenu
 * \see updateToolBar
 */
void qmdiActionGroupList::mergeGroupList(qmdiActionGroupList *group) {
    foreach (qmdiActionGroup *i, group->actionGroups) {
        qmdiActionGroup *mine = getActionGroup(i->getName());
        mine->mergeGroup(i);
    }
}

/**
 * \brief un-merge an action group list
 * \param group the old group to remove from this action group list
 *
 * This function removes external definitions from this action group list.
 * If at the end of the un-merge some action groups are empty, \b they \b will
 * \b not \b be \b removed \b from \b this \b class. Since the generation of
 * menus (using updateMenu() ) does not include empty menus, this is totally
 * acceptable
 *
 * Note that just un-merging an action group list will not totally reflect the
 * GUI, and you might also need to update the real widget which this action
 * group list represents.
 *
 * \see mergeGroupList
 * \see updateMenu
 * \see updateToolBar
 */
void qmdiActionGroupList::unmergeGroupList(qmdiActionGroupList *group) {
    foreach (qmdiActionGroup *i, group->actionGroups) {
        qmdiActionGroup *mine = getActionGroup(i->getName());
        mine->unmergeGroup(i);
    }
}

/**
 * \brief update a QMenuBar from the definitions on this action group list
 * \param menubar a QMenuBar to be updated
 * \return the updated menubar (same instance which was passed)
 *
 * This function generates from the definitions on this class a valid
 * QMenuBar which will be showed on a QMainWindow.
 *
 * If \c menubar is nullptr, a new QMenuBar will be allocated for you, and
 * will be returned.
 *
 * You cannot generate items into a QMenuBar "by hand" and then "add"
 * the definitions on this class.
 */
QMenuBar *qmdiActionGroupList::updateMenuBar(QMenuBar *menubar) {
    if (menubar)
        menubar->clear();
    else
        menubar = new QMenuBar(menubar);

    foreach (qmdiActionGroup *i, actionGroups) {
        QMenu *m = i->updateMenu();

        if (m)
            menubar->addMenu(m);
    }

    return menubar;
}

/**
 * \brief Deprecated - use updateMenuBar()
 * \param menubar the menubar to be updated
 * \return an updated menubar (same as the one passed as a parameter)
 *
 *
 * This method is deprecated, please use updateMenuBar() instead.
 */
QMenuBar *qmdiActionGroupList::updateMenu(QMenuBar *menubar) {
    qWarning("This qmdiActionGroupList::updateMenu() is deprecated, please use "
             "qmdiActionGroupList::updateMenuBar() instead");
    return updateMenuBar(menubar);
}

/**
 * \brief update a QMenu from the definitions on this action group list
 * \param popupMenu menubar a QMenuBar to be updated
 * \return the updated menubar (same instance which was passed)
 *
 * This function generates from the definitions on this class a valid
 * QMenur which you can show on you widget as a context menu for example.
 *
 * If \c popupMenu is nullptr, a new QMenu will be allocated for you, and
 * will be returned. This method is similar to updateMenuBar() with
 * the difference that the actio group list is converted to a single popup menu.
 *
 */
QMenu *qmdiActionGroupList::updatePopMenu(QMenu *popupMenu) {
    if (popupMenu)
        popupMenu->clear();
    else
        popupMenu = new QMenu(popupMenu);

    foreach (qmdiActionGroup *i, actionGroups) {
        QMenu *m = i->updateMenu();

        if (m)
            popupMenu->addMenu(m);
    }

    return popupMenu;
}

/**
 * \brief update a list of QToolBars from the definitions on this action group
 * list \param window the window in which the toolbars should be place \param
 * toolbars a list o toolbars available on the main window \return a list of
 * toolbars which has been created from this action group list
 *
 * This function generates from the definitions on this class a valid list of
 * QToolBar which will be showed on the \c window .
 *
 * If the \c toolbars array will be nullptr, a new one will be allocated for
 * you. When merging toolbars if a break is defined, a break will be added by
 * this method.
 *
 * While you can add toolbars "manually" to your main window, it's not
 * recommended because new actions will not get merged into your toolbar.
 * Instead you might get 2 toolbars with a similar name.
 */
QList<QToolBar *> *qmdiActionGroupList::updateToolBar(QList<QToolBar *> *toolbars,
                                                      QMainWindow *window) {
    if (toolbars == nullptr)
        toolbars = new QList<QToolBar *>;

    foreach (qmdiActionGroup *i, actionGroups) {
        QToolBar *tb = nullptr;
        QString actionName = i->getName();

        // find the correct toolbar
        foreach (QToolBar *b, *toolbars) {
            if (b->windowTitle() == actionName) {
                tb = b;
                break;
            }
        }

        // if none found, create one
        if (tb == nullptr) {
            tb = new QToolBar(actionName, window);
            tb->setObjectName(actionName);
            *toolbars << tb;
            window->addToolBar(tb);
            if (i->breakAfter)
                window->addToolBarBreak();
        }

        // merge it with the corresponding group list
        tb = i->updateToolBar(tb);
    }

    return toolbars;
}
