#include <QMainWindow>

#include "qmditabwidget.h"
#include "qmdihost.h"
#include "qmdiclient.h"

/**
 * \file qmditabwidget.cpp
 * \brief Implementation of the qmdi tab widget
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiServer, QTabWidget
 */


qmdiTabWidget::qmdiTabWidget( QWidget *parent )
	: QTabWidget( parent )
{
	mdiHost = dynamic_cast<qmdiHost*>(parent);
	activeWidget = NULL;

	connect( this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

void qmdiTabWidget::tabChanged( int i )
{
	if (mdiHost == NULL)
		return;
		
	qmdiClient *client = NULL;
	QWidget *w = widget( i );

	// nothing to do, if the same tab has been selected twise
	if (w == activeWidget)
		return;

	if (activeWidget)
	{
		client = dynamic_cast<qmdiClient*>(activeWidget);
		if (client != NULL)
		{
			mdiHost->menus.unmergeGroupList( &client->menus );
			mdiHost->toolbars.unmergeGroupList( &client->toolbars );
		}
		else
			qDebug("%s: no found client on removing", __func__ );
	}
	
	activeWidget = w;	
	if (activeWidget)
	{
		client = dynamic_cast<qmdiClient*>(activeWidget);
		if (client != NULL)
		{
			mdiHost->menus.mergeGroupList( &client->menus );
			mdiHost->toolbars.mergeGroupList( &client->toolbars );
		}
		else
			qDebug("%s: no found client on merging", __func__  );
	}

	QMainWindow *m = dynamic_cast<QMainWindow*>(mdiHost);
	mdiHost->toolBarList = mdiHost->toolbars.updateToolBar( mdiHost->toolBarList, m );
	mdiHost->menus.updateMenu( m->menuBar() );
}

void qmdiTabWidget::clientDeleted( QObject *o )
{
	if (o == NULL)
		return;
		
	if (mdiHost == NULL)
		return;
		
	if (activeWidget != o)
		return;

	qmdiClient *client = dynamic_cast<qmdiClient*>(activeWidget);
	if (client != NULL)
	{
		mdiHost->menus.unmergeGroupList( &client->menus );
		mdiHost->toolbars.unmergeGroupList( &client->toolbars );
	}
	activeWidget = NULL;
	
	QMainWindow *m = dynamic_cast<QMainWindow*>(mdiHost);
	mdiHost->toolBarList = mdiHost->toolbars.updateToolBar( mdiHost->toolBarList, m );
	mdiHost->menus.updateMenu( m->menuBar() );
}

void qmdiTabWidget::tabInserted ( int index )
{
	QWidget *w = widget( index );
	qmdiClient *client = dynamic_cast<qmdiClient*>(w);

	if (client != NULL)
	{
		client->mdiServer = dynamic_cast<qmdiServer*>(this);
		client->myself = w;
	}

//	if it's the only widget available, show it's number
//	BUG is this supposed to be done by Qt?
	if (count() == 1)
		tabChanged( 0 );
}

void qmdiTabWidget::tabRemoved ( int index )
{
	if (mdiHost == NULL)
		return;

	// this is done to shut up gcc warnings
	index = 0;
	if (count() == 0)
	{
		tabChanged( 0 );
		activeWidget = NULL;

		// the deletion of menus and toolbars is made by qmdiClient itself
		QMainWindow *m = dynamic_cast<QMainWindow*>(mdiHost);
		mdiHost->toolBarList = mdiHost->toolbars.updateToolBar( mdiHost->toolBarList, m );
		mdiHost->menus.updateMenu( m->menuBar() );
	}
}
