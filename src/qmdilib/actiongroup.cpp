/**
 * \file actiongroup.cpp
 * \brief Implementation of the action group class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiActionGroup
 */

#include <QAction>
#include <QMenu>
#include <QToolBar>

#include "actiongroup.h"

/**
 * \class qmdiActionGroup
 * \brief an abstruction layer for QMenu and QToolBar
 *
 * This class defines the items that you see on a QMenu and
 * QToolBar, with a much simplified interface. This class has
 * the ability to merge two menus, and thus allowing the new menu
 * to overwrite the actions of the orignal one.
 * 
 * The action group has a name, which will be used for creating a
 * popup menu on a QMenuBar, or setting the toolbar name.
 *
 * \see qmdiActionGroupList
 * \see getName
 */


/**
 * Default constructor. Builds a new qmdiActionGroup
 * with a give name. The action group will contain no 
 * actions by default, representing an empty menu or toolbar.
 */
qmdiActionGroup::qmdiActionGroup( QString name )
{
	this->name = name;
}


/**
 * Empty destructor. Destroyes the object.
 */
qmdiActionGroup::qmdiActionGroup()
{
}

qmdiActionGroup::~qmdiActionGroup()
{
	// TODO delete all
}

/**
 * \brief sets an name for this action group
 * \param name the new name for the action group
 * 
 * Sets the name for the action group. The name will be used
 * for describing the toolbar or menu item, and thus is very
 * important to set it correctly.
 * 
 * \see getName
 */
void qmdiActionGroup::setName( QString name )
{
	this->name = name;
}

/**
 * \brief returns the name of the action group
 * \return the name of the action group
 * 
 * Gets the name for the action group. The name will be used
 * for describing the toolbar or menu item, and thus is very
 * important to set it correctly.
 */
QString qmdiActionGroup::getName()
{
	return name;
}

/**
 * \brief add a new action to the action group
 * \param action item to be added to the action group
 * 
 * When calling this function, you are adding a new 
 * item to the toolbar or menu represented by the action
 * group.
 * 
 * Actions are added to the end of the list. There is no way
 * to reorder the actions once they are in the group.
 * 
 * \see addSeparator
 * \see containsAction
 * \see removeAction
 */
void qmdiActionGroup::addAction( QAction *action )
{
	if (containsAction(action))
		removeAction( action );

	actionGroupItems << action;
}

/**
 * \brief adds a separator to the menu or toolbar
 * 
 * This function will add a separator to the menu or 
 * toolbar reporesented by this action group.
 * 
 * \see addAction
 * \see removeAction
 */
void qmdiActionGroup::addSeparator()
{
	QAction *separator = new QAction( NULL );
	separator->setSeparator( true );

	addAction( separator );
}

/**
 * \brief returns if an action is found in this group
 * \param action QAction to be tested
 * \return true if the action is found in this group action
 * 
 * Use this function for testing if some action is fonud on
 * the action group.
 */
bool qmdiActionGroup::containsAction( QAction *action )
{
	return actionGroupItems.contains( action );
}

/**
 * \brief remove an action from the action group
 * \param action QAction item to be removed
 * 
 * Use this function for removing items from the menu or 
 * toolbar reporesented by this action group.
 * 
 * \see addAction
 */
void qmdiActionGroup::removeAction( QAction *action )
{
	int i =	actionGroupItems.indexOf( action );

	if ( i != -1 )
		actionGroupItems.removeAt( i );
}


/**
 * \brief merges another action group actions into this action group
 * \param group the new group to be merged
 * 
 * Use this call if you want to merge the items of another group into
 * one. The actions of the new group will be placed at the end of the
 * list of actions available on this
 * 
 * \see unmergeGroup
 */
void qmdiActionGroup::mergeGroup( qmdiActionGroup *group )
{
	if (!group)
		return;
		
	foreach( QObject *o, group->actionGroupItems )
	{
		QAction *a = qobject_cast<QAction*> (o);
		if (a)
			addAction( a );
	}	
}


/**
 * \brief unmerges another action group actions into this action group
 * \param group the group to be removed from this group
 * 
 * Use this call if you want to unmerge the items of another group into
 * one.
 * 
 * \see mergeGroup
 */
void qmdiActionGroup::unmergeGroup( qmdiActionGroup *group )
{
	if (!group)
		return;
		
	foreach( QObject *o, group->actionGroupItems )
	{
		QAction *a = qobject_cast<QAction*> (o);
		if (a)
			removeAction( a );
	}	
}

/**
 * \brief generates an updated menu from the items on the group list
 * \param menu a
 * \return an updated menu
 *
 * Call this function to update a QMenu from these definitions.
 * If \param menu is \b NULL then a new menu will be allocated.
 *
 * The returned value is not unallocated by this function, and it's
 * up to the programmer to unallocate the memory used by the created menu.
 *
 * If you are inserting that QMenu into a QMenuBar the memory deallocation
 * will be handeled by QMenuBar, and you don't have to bother about it.
 * 
 * If the action group contains no items, no menu will be generated, and 
 * NULL will be the returned value. If the passed \param menu is not NULL
 * it will be deallocated.
 *
 * \see updateToolBar
 */
QMenu*	 qmdiActionGroup::updateMenu( QMenu *menu )
{
	if (actionGroupItems.isEmpty())
	{
		delete menu;
		return NULL;
	}
	
	if (!menu)
		menu = new QMenu( name );

	menu->clear();
	
	foreach( QObject *o, actionGroupItems )
	{
		QAction *a = qobject_cast<QAction*> (o);
		if (a)
			menu->addAction( a );
	}

	return menu;
}


/**
 * \brief generates an updated toolbar from the items on the group list
 * \param toolbar the toolbar to update
 * \return an updated toolbar
 *
 * Call this function to update a QToolBar from these definitions.
 * If \param toolbar is \b NULL then a new toolbar will be allocated.
 *
 * The returned value is not unallocated by this function, and it's
 * up to the programmer to unallocate the memory used by the created menu.
 *
 * If you are inserting that QToolBar into a QMainWindow the memory deallocation
 * will be handeled by QMainWindow, and you don't have to bother about it.
 *
 * \see updateMenu
 */
QToolBar* qmdiActionGroup::updateToolBar( QToolBar *toolbar )
{
	if (!toolbar)
		toolbar = new QToolBar( name );

	toolbar->clear();
	
	foreach( QObject *o, actionGroupItems )
	{
		QAction *a = qobject_cast<QAction*> (o);
		if (a)
			toolbar->addAction( a );
	}

	if (actionGroupItems.count() == 0)
		toolbar->hide();
	else
		toolbar->show();
	
	return toolbar;
}
