#ifndef __ACTION_GROUP_H__
#define __ACTION_GROUP_H__

/**
 * \file actiongroup.h
 * \brief Definition of the action group class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiActionGroup
 */
 
#include <QList>
#include <QString>

class QAction;
class QObject;
class QMenu;
class QToolBar;

class qmdiActionGroup
{
public:
	qmdiActionGroup( QString name );
	qmdiActionGroup();
	~qmdiActionGroup();

	void		setName( QString name );
	QString		getName();
	void		addAction( QAction *action );
	void		addSeparator();
	bool		containsAction( QAction *action );
	void		removeAction( QAction *action );

	void		mergeGroup( qmdiActionGroup *group );
	void		unmergeGroup( qmdiActionGroup *group );

	void		clear();

	QMenu*		updateMenu( QMenu *menu=NULL );
	QToolBar*	updateToolBar( QToolBar *toolbar );
	
private:
	QString name;
	QList<QObject*> actionGroupItems;
};
#endif //__ACTION_GROUP__
