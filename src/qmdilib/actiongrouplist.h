#ifndef __ACTION_GROUP_LIST_H__
#define __ACTION_GROUP_LIST_H__

/**
 * \file actiongrouplist.h
 * \brief Definition of the action group list class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiActionGroupList
 */

#include <QList>
#include "actiongroup.h"

class QObject;
class QAction;
class QString;
class QMenuBar;
class QMainWindow;

class qmdiActionGroupList
{
public:
	qmdiActionGroupList();
		
	qmdiActionGroup* operator[]( const QString name );
	qmdiActionGroup* getActionGroup( const QString name );
	void mergeGroupList( qmdiActionGroupList *group );
	void unmergeGroupList( qmdiActionGroupList *group );
	void clear();
	
	QMenuBar*		updateMenu( QMenuBar *menubar );
	QList<QToolBar*>*	updateToolBar( QList<QToolBar*> *toolbars, QMainWindow *window );
	
private:
	QList<qmdiActionGroup*> actionGroups;
};

#endif //__ACTION_GROUP__
