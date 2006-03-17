#include <QString>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QMainWindow>

#include "actiongrouplist.h"

/**
 * \file actiongrouplist.cpp
 * \brief Implementation of the action group list class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiActionGroupList
 */


/**
 * \class qmdiActionGroupList
 * \brief abstraction layer for QMenuBar and a list of toolbars
 *
 * This class defines a QMenuBar and the list of toolbars available on
 * a tipical application. Each submenu or toolbar is defined by one 
 * qmdiActionGroup. 
 * 
 * This class has also the ability to merge other qmdiActionGroupList (this enables
 * widgets to add their partial menus to the menus supplied by the main application).
 */


/**
 * Build an empty action group list. If you generate a menubar 
 * from this empty class, you will get a NIL menu. Generating
 * a toolbar set from this empty class will generate no toolbars.
 */
qmdiActionGroupList::qmdiActionGroupList()
{
}


/**
 * \brief overloaded operator for getting the instance of a action group
 * \param name the action group name you want to get
 * \return an instace to an action group
 * 
 * This is just an overloaded function which calls getActionGroup().
 * 
 * \see getActionGroup()
 */
qmdiActionGroup* qmdiActionGroupList::operator[]( const QString name )
{
	return getActionGroup( name );
}


/**
 * \brief get the instance of a action group
 * \param name the action group name you want to get
 * \return an instace to an action group
 * 
 * This function returns an instace to a action group. Action groups
 * are abstractions of QMenu and QToolBar.
 * 
 * If the action group requested is not available, a new instace will be
 * created.
 * 
 * \see updateMenu()
 * \see updateToolBar()
 */
qmdiActionGroup* qmdiActionGroupList::getActionGroup( const QString name )
{
        qmdiActionGroup *item = NULL;

	foreach( qmdiActionGroup* i, actionGroups )
	{
		if (i->getName() == name )
			return i;
	}
	
	// if menu does not exist, create it
	item = new qmdiActionGroup( name );
	actionGroups.append( item );
	return item;
}

/**
 * \brief merge another action group list
 * \param group the new group to merge into this one
 * 
 * This function merges an action group list definition into this
 * action group list:
 *  - If in the new group there are action groups, the items will be appended to the existing ones
 *  - If in the new group there are new actions groups, those groups will be added to this action group list
 *   
 * Note that just merging is not enough, and you might need also to update 
 * the real widget which this action group list represents.
 * 
 * \see unmergeGroupList
 * \see updateMenu
 * \see updateToolBar
 */
void qmdiActionGroupList::mergeGroupList( qmdiActionGroupList *group )
{
	foreach( qmdiActionGroup* i, group->actionGroups )
	{
		qmdiActionGroup *mine = getActionGroup( i->getName() );
		mine->mergeGroup( i );
	}
}


/**
 * \brief unmerge an action group list
 * \param group the old group to remove from this action group list
 * 
 * This function removes external definitions from this action group list.
 * If at the end of the unmerge, some action groups are empty, \b they \b will
 * \b not \b be \b removed \b from \b this \b class. Since the generation of menus
 * (using updateMenu() ) does not include empty menus, this is totally accepatable.
 * 
 * Note that just unmerging an action group list will not totally reflect the GUI,
 * and you might also need to update the real widget which this action group list represents.
 * 
 * \see mergeGroupList
 * \see updateMenu
 * \see updateToolBar
 */
void qmdiActionGroupList::unmergeGroupList( qmdiActionGroupList *group )
{
	foreach( qmdiActionGroup* i, group->actionGroups )
	{
		qmdiActionGroup *mine = getActionGroup( i->getName() );
		mine->unmergeGroup( i );
	}
}


/**
 * \brief update a QMenuBar from the definitions on this action group list
 * \param menubar a QMenuBar to be updated
 * \return the updated menubar (same instace which was passed)
 * 
 * This function generates from the definitions on this class a valid
 * QMenuBar which will be showed on a QMainWindow. 
 * 
 * If \c menubar is NULL, a new QMenuBar will be allocated for you, and
 * will be returned.
 * 
 * You cannot generate items into a QMenuBar "by hand" and then "add"
 * the definitions on this class. 
 */
QMenuBar* qmdiActionGroupList::updateMenu( QMenuBar *menubar )
{
	if (menubar)
		menubar->clear();
	else
		menubar = new QMenuBar();

	foreach( qmdiActionGroup* i, actionGroups )
	{
		QMenu *m = i->updateMenu();

		if (m)
			menubar->addMenu( m );
	}

	return menubar;
}


/**
 * \brief update a list of QToolBars from the definitions on this action group list
 * \param window the window in which the toolbars should be placed
 * \return
 *
 */
QList<QToolBar*>* qmdiActionGroupList::updateToolBar( QList<QToolBar*> *toolbars, QMainWindow *window )
{
	if (toolbars == NULL)
		toolbars = new QList<QToolBar*>;

	foreach( qmdiActionGroup* i, actionGroups )
	{
		QToolBar *tb = NULL;
		QString  actionName = i->getName();
		
		// find the correct toolbar
		foreach( QToolBar *b, *toolbars )
		{
			if (b->windowTitle() == actionName)
			{
				tb = b;
				break;
			}
		}

		// if none found, create one
		if (tb == NULL)
		{
			tb = new QToolBar( actionName );
			tb->setObjectName( actionName );
			*toolbars << tb;
			window->addToolBar( tb );
		}
		
		// merge it with the corresponding group list
		tb = i->updateToolBar( tb );
	}

	return toolbars;
}
