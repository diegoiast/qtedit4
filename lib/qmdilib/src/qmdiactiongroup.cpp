/**
 * \file qmdiactiongroup.cpp
 * \brief Implementation of the action group class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiActionGroup
 */

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>

#include "qmdiactiongroup.h"

/**
 * \class qmdiActionGroup
 * \brief an abstraction layer for QMenu and QToolBar
 *
 * This class defines the items that you see on a QMenu and
 * QToolBar, with a much simplified interface. This class has
 * the ability to merge two menus, and thus allowing the new menu
 * to overwrite the actions of the original one.
 *
 * The action group has a name, which will be used for creating a
 * pop-up menu on a QMenuBar, or setting the toolbar name.
 *
 * \see qmdiActionGroupList
 * \see getName()
 */

/**
 * \var qmdiActionGroup::name
 * \brief the name of the group
 *
 * The name of the group, as passed to the constructor. This should not be
 * changed, since it also defined the text to be seen on the menu of toolbar
 * created by this class.
 *
 * \internal
 *
 * \see getName()
 * \see setName()
 */

/**
 * \var qmdiActionGroup::breakAfter
 * \brief defined if after mergeing, a break should be put
 *
 * This property defines if after merging this group there should be a break.
 * This is used by qmdiActionGroupList::updateToolBar()
 *
 * \copydoc read_only_after_constructor
 *
 * \see qmdiActionGroupList::updateToolBar()
 */

/**
 * \var qmdiActionGroup::actionGroupItems
 * \brief sub items in this list
 *
 * A list of \b QAction or \b QWidget which shuold be displayed on the toolbars
 * or menus created by this list. Items can be put into this list by addding
 * them directly (using addAction() or addWidget() ) or by merging in another
 * action group.
 *
 * \internal
 * \see mergeGroup()
 * \see addAction()
 * \see addWidget()
 */

/**
 * \var qmdiActionGroup::actionGroups
 * \brief other groups available in this group
 *
 * List of merged in groups.
 *
 * \internal
 * \see mergeGroup()
 */

/**
 * \var qmdiActionGroup::breakCount
 * \brief
 *
 * \todo document this variable
 * \internal
 */

/**
 * \var qmdiActionGroup::mergeLocation
 * \brief the location where to merge in new sub groups
 *
 * This defined the location in which to merge in actions of merged in groups.
 *
 * \internal
 * \see mergeGroup()
 */

/**
 * \brief Constructs an MDI action group
 * \param name the name of the action group
 *
 * Default constructor. Builds a new qmdiActionGroup
 * with a give name. The action group will contain no
 * actions by default, representing an empty menu or toolbar.
 *
 * If you generate a menu from this action group, the
 * \b name will be used as the title of the menu.
 *
 * If you generate a toolbar from this action group, the
 * \b name will be used as title of this toolbar.
 *
 * \see updateMenu()
 * \see updateToolBar()
 *
 * \todo How about localization? maybe we need to differentiate between name and
 * title?
 */
qmdiActionGroup::qmdiActionGroup(QString name) {
    this->name = name;

    breakAfter = false;
    breakCount = -1;
    mergeLocation = -1;
}

/**
 * Empty destructor. Destroys the object.
 */
qmdiActionGroup::~qmdiActionGroup() {
    // TODO should we delete all ?
}

/**
 * \var qmdiActionGroup::breakAfter
 * \brief Defines if a break should be added after this action group list
 *
 * If you set this property to \b true , and you are generating a toolbar
 * from this action group list, when the toolbar will be displayed for the
 * fist time at the screen a break will be entered after this tool.
 *
 * This will actually  call:
 * \code
 * QMainWindow::addToolBarBreak()
 * \endcode
 *
 * \copydoc read_only_after_constructor
 */

/**
 * \brief sets an name for this action group
 * \param newName the new name for the action group
 *
 * Sets the name for the action group. The name will be used
 * for describing the toolbar or menu item, and thus is very
 * important to set it correctly.
 *
 * This will not modify the menu or toolbar text - please call the correct
 * update method to do that.
 *
 * \see getName
 * \see updateMenu()
 * \see updateToolBar()
 */
void qmdiActionGroup::setName(const QString &newName) { this->name = newName; }

/**
 * \brief returns the name of the action group
 * \return the name of the action group
 *
 * Gets the name for the action group. The name will be used
 * for describing the toolbar or menu item, and thus is very
 * important to set it correctly.
 */
QString qmdiActionGroup::getName() { return name; }

/**
 * \brief add a new action to the action group
 * \param action item to be added to the action group
 * \param location where to add the new action
 *
 * When calling this function, you are adding a new
 * item to the toolbar or menu represented by the action
 * group.
 *
 * The action is added to the end of the list, if the location  parameter is -1
 * or the mergepoint, otherwise the location specifies where the actions are
 * added.
 *
 * There is no way to reorder the actions once they are in the group.
 *
 * \see addSeparator()
 * \see addMenu()
 * \see containsAction()
 * \see removeAction()
 * \see setMergePoint()
 */
void qmdiActionGroup::addAction(QAction *action, int location) {
    if (containsAction(action))
        removeAction(action);

    if (location != -1)
        actionGroupItems.insert(location, action);
    else
        actionGroupItems << action;
}

/**
 * \brief add new actions to the action group
 * \param actions items to be added to the action group
 * \param location where to add the new actiongroup
 * \since 0.0.4
 *
 * Overloaded function for convience.
 *
 * \see addAction
 */
void qmdiActionGroup::addActions(QActionGroup *actions, int location) {
    if (location == -1)
        location = actionGroupItems.count();
    foreach (QAction *a, actions->actions()) {
        addAction(a, location);
        location++;
    }
}

/**
 * \brief add a new widget to the action group
 * \param widget item to be added to the action group
 * \param location where to add the new widget
 *
 * When calling this function, you are adding a new
 * widget to the toolbar or menu represented by the action
 * group.
 *
 * Widget is added to the end of the list if location is -1,
 * otherwise the location specifies where the widget is added.
 * There is no way to reorder the widgets once they are in the group.
 * If you are are generating a menu, this widget is ignored.
 *
 * \see removeWidget
 * \see updateMenu
 * \see updateToolBar
 * \see setMergePoint
 */
void qmdiActionGroup::addWidget(QWidget *widget, int location) {
    if (location != -1)
        actionGroupItems.insert(location, widget);
    else
        actionGroupItems << widget;
}

/**
 * \brief adds a submenu to the menu or toolbar
 * \param menu item to be added to the action group
 * \param location  where to add the new widget
 * \since 0.0.4
 *
 * If you want to add sub menus to some menu bars, you should
 * use this function.
 *
 * The menu is added to the end of the list if location is -1,
 * otherwise the location specifies where the menu is added.
 *
 * If you are are generating a toolbar, this menu is ignored.
 *
 * \see updateToolBar
 * \see updateMenu
 * \see addWidget
 * \see setMergePoint
 */
void qmdiActionGroup::addMenu(QMenu *menu, int location) {
    if (location != -1)
        actionGroupItems.insert(location, menu);
    else
        actionGroupItems << menu;
}

/**
 * \brief adds a separator to the menu or toolbar
 * \param location where to add the new widget
 *
 * This function will add a separator to the menu or
 * toolbar represented by this action group. The
 * separator will be added to the end of the list,
 * unless a location is not -1.
 *
 * \see addAction
 * \see removeAction
 * \see setMergePoint
 */
void qmdiActionGroup::addSeparator(int location) {
    QAction *separator = new QAction(nullptr);
    separator->setSeparator(true);

    addAction(separator, location);
}

/**
 * \brief returns if an action is found in this group
 * \param action QAction to be tested
 * \return true if the action is found in this group action
 *
 * Use this function for testing if some action is found on
 * the action group.
 */
bool qmdiActionGroup::containsAction(QAction *action) { return actionGroupItems.contains(action); }

/**
 * \brief remove an action from the action group
 * \param action QAction item to be removed
 *
 * Use this function for removing items from the menu or
 * toolbar represented by this action group.
 *
 * \see addAction
 */
void qmdiActionGroup::removeAction(QAction *action) {
    int i = actionGroupItems.indexOf(action);

    if (i != -1)
        actionGroupItems.removeAt(i);
}

/**
 * \brief remove a actions from the action group
 * \param actions items to be removed
 * \since 0.0.4
 *
 * Use this function for removing items from the menu or
 * toolbar represented by this action group.
 *
 * \see removeAction
 * \see addAction
 */
void qmdiActionGroup::removeActions(QActionGroup *actions) {
    foreach (QAction *a, actions->actions()) {
        removeAction(a);
    }
}

/**
 * \brief remove a menu from the action group
 * \param menu menu item to be removed
 *
 * Use this function for removing sub-menus from the menu represented
 * by this action group.
 *
 * \see removeAction
 * \see addAction
 * \see addWidget
 * \see addMenu
 * \see updateMenu
 */
void qmdiActionGroup::removeMenu(QMenu *menu) {
    int i = actionGroupItems.indexOf(menu);

    if (i != -1)
        actionGroupItems.removeAt(i);
}

/**
 * \brief remove a widget from the action group
 * \param widget QWidget item to be removed
 *
 * Use this function for removing widgets from the menu or
 * toolbar represented by this action group.
 *
 * \see addAction
 * \see removeAction
 * \see addWidget
 * \see updateMenu
 * \see updateToolBar
 */
void qmdiActionGroup::removeWidget(QWidget *widget) {
    int i = actionGroupItems.indexOf(widget);

    if (i != -1)
        actionGroupItems.removeAt(i);
}

/**
 * \brief set the location for menus and toolbar merges
 * \since 0.0.3
 *
 * By default, an action group gets merged to the end of
 * the parent action group. Using this function, you can define a new
 * location to merge new action groups.
 *
 */
void qmdiActionGroup::setMergePoint() { mergeLocation = actionGroupItems.count(); }

/**
 * \brief compute the best merging point for new action groups
 * \since 0.0.4
 * \return -1 is not defined, otherwise a position lower then count()
 *
 * This function computes the best location in which a new action group
 * should be merged. The merge point is computed from the list of
 * merged action groups - the last added action group will win.
 *
 * If no merging point is defined, the default is to merge at the top
 * of menu or toolbar.
 *
 * \todo action groups should also have merging priorities
 * \see mergeGroup
 */
int qmdiActionGroup::getMergePoint() {
    int i = -1;

    foreach (qmdiActionGroup *ag, actionGroups) {
        if (ag->mergeLocation > i)
            i = ag->mergeLocation;
    }

    if (mergeLocation > i)
        i = mergeLocation;

    return i;
}

/**
 * \brief merges another action group actions into this action group
 * \param group the new group to be merged
 *
 * Use this call if you want to merge the items of another group into
 * one. The actions of the new group will be placed at the end of the
 * list of actions available on this, unless a merge location as been
 * defined.
 *
 * The merge point is calculated from the list of merged action groups
 * or the self defined action group. For more documentation see the
 * documentation of getMergePoint.
 *
 * \see unmergeGroup
 * \see setMergePoint
 * \see getMergePoint
 */
void qmdiActionGroup::mergeGroup(qmdiActionGroup *group) {
    if (!group)
        return;

    if ((group->breakAfter))
        breakCount = breakCount == -1 ? 1 : breakCount + 1;

    int i = 0;
    foreach (QObject *o, group->actionGroupItems) {
        if (actionGroupItems.contains(o))
            continue;

        QAction *a = qobject_cast<QAction *>(o);

        if (a) {
            int m = getMergePoint();

            if (m != -1)
                addAction(a, m + i);
            else
                addAction(a);
        } else {
            QWidget *w = qobject_cast<QWidget *>(o);
            if (w) {
                int m = getMergePoint();

                if (m != -1)
                    addWidget(w, m + i);
                else
                    addWidget(w);

                // don't display menus, as they are displayed on demand
                // when selected in the QMainMenu
                if (!w->inherits("QMenu"))
                    w->setVisible(true);
            } else
                qDebug("%s %d : erorr - wrong QObject type added to action group", __FILE__,
                       __LINE__);
        }

        i++;
    }

    if (breakCount > 0)
        breakAfter = true;

    actionGroups << group;
}

/**
 * \brief un-merges another action group actions into this action group
 * \param group the group to be removed from this group
 *
 * Use this call if you want to un-merge the items of another group into
 * one.
 *
 * \see mergeGroup
 */
void qmdiActionGroup::unmergeGroup(qmdiActionGroup *group) {
    if (!group)
        return;

    if ((group->breakAfter))
        breakCount = breakCount > 1 ? -1 : breakCount - 1;

    foreach (QObject *o, group->actionGroupItems) {
        if (!actionGroupItems.contains(o))
            continue;

        QAction *a = qobject_cast<QAction *>(o);
        if (a)
            removeAction(a);
        else {
            QWidget *w = qobject_cast<QWidget *>(o);
            if (w) {
                w->setVisible(false);
                removeWidget(w);
            }
        }
    }

    if (breakCount > 0)
        breakAfter = true;

    if (actionGroups.contains(group))
        actionGroups.removeAt(actionGroups.indexOf(group));
}

/**
 * \brief generates an updated menu from the items on the group list
 * \param menu a
 * \return an updated menu
 *
 * Call this function to update a QMenu from these definitions.
 * If \b menu is \b nullptr then a new menu will be allocated.
 *
 * The returned value is not unallocated by this function, and it's
 * up to the programmer to un-allocate the memory used by the created menu.
 *
 * If you are inserting that QMenu into a QMenuBar the memory deallocation
 * will be handled by QMenuBar, and you don't have to bother about it.
 *
 * If the action group contains no items, no menu will be generated, and
 * nullptr will be the returned value. If the passed \b menu is not nullptr
 * it will be deallocated.
 *
 * \see updateToolBar
 */
QMenu *qmdiActionGroup::updateMenu(QMenu *menu) {
    if (actionGroupItems.isEmpty()) {
        delete menu;
        return nullptr;
    }

    if (!menu)
        menu = new QMenu(name);
    else
        menu->setTitle(name);
    menu->clear();

    foreach (QObject *o, actionGroupItems) {
        QAction *a = qobject_cast<QAction *>(o);
        if (a)
            menu->addAction(a);

        QMenu *m = qobject_cast<QMenu *>(o);
        if (m)
            menu->addMenu(m);
    }

    return menu;
}

/**
 * \brief generates an updated toolbar from the items on the group list
 * \param toolbar the toolbar to update
 * \return an updated toolbar
 *
 * Call this function to update a QToolBar from these definitions.
 * If \param toolbar is \b nullptr then a new toolbar will be allocated.
 *
 * The returned value is not unallocated by this function, and it's
 * up to the programmer to un-allocate the memory used by the created menu.
 *
 * If you are inserting that QToolBar into a QMainWindow the memory deallocation
 * will be handled by QMainWindow, and you don't have to bother about it.
 *
 * \see updateMenu
 */
QToolBar *qmdiActionGroup::updateToolBar(QToolBar *toolbar) {
    if (!toolbar)
        toolbar = new QToolBar(name);
    else
        toolbar->setWindowTitle(name);

    toolbar->setUpdatesEnabled(false);
    toolbar->hide();
    toolbar->clear();
    int i = 0;
    foreach (QObject *o, actionGroupItems) {
        QAction *a = qobject_cast<QAction *>(o);
        if (a)
            toolbar->addAction(a);
        else {
            QWidget *w = qobject_cast<QWidget *>(o);
            if (w) {
                // don't even try to add menus to toolbars, this just does not work
                if (!w->inherits("QMenu"))
                    toolbar->addWidget(w);
            }
        }
        i++;
    }

    if (actionGroupItems.count() == 0)
        toolbar->hide();
    else
        toolbar->show();
    toolbar->setUpdatesEnabled(true);

    return toolbar;
}
