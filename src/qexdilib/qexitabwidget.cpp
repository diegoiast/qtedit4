#include <QtDebug>

#include <QMainWindow>
#include <QMenuBar>

#include "qexitabwidget.h"


/** 
  * \class QexMenuItemList
  * \brief Abstruction layer for QMenu
  */
QexMenuItemList::QexMenuItemList( QString n )
{
	name = n;
}

QexMenuItemList::~QexMenuItemList()
{
}

void	QexMenuItemList::add( QAction *action )
{
	if (contains(action))
		remove( action );

	menus.append( action );
}

void	QexMenuItemList::addSeparator()
{
	QAction *a = new QAction( NULL );
	a->setSeparator( true );
	
	menus.append( a );
}

void	QexMenuItemList::remove( QAction *action )
{
	menus.removeAll( action );
}

bool	QexMenuItemList::contains( QAction *action )
{
	if (action == NULL)
		return false;

	return menus.contains( action );
}

int 	QexMenuItemList::count()
{
	return menus.count();
}

/**
 * \class QexMenuList
 * \brief Abstruction layer for QMenuBar
 * 
 */
QexMenuList::QexMenuList()
{
}

QexMenuList::~QexMenuList()
{
}

void	QexMenuList::add( QString menuName, QAction *action )
{
	if (contains(action))
		remove( action );
	
	// check for menus
	QexMenuItemList *i = getMenu( menuName );
	if ( i == NULL )
	{
		// if menu does not exist, create it
		i = new QexMenuItemList( menuName );
		menuItemList.append( i );
	}

	// insert action into menu
	i->add( action );
}

void	QexMenuList::remove( QAction *action )
{
	foreach( QexMenuItemList* i, menuItemList )
	{
		if (i->contains(action))
			i->remove( action );
	}
}

bool	QexMenuList::contains( QAction *action )
{
	foreach( QexMenuItemList* i, menuItemList )
	{
		if (i->contains(action))
			return true;
	}
	return false;
}

QexMenuItemList* QexMenuList::getMenu( QString name )
{
	QexMenuItemList *item = NULL;

	foreach( QexMenuItemList* i, menuItemList )
	{
		if (i->getName() == name )
			return i;
	}
	
	// if menu does not exist, create it
	item = new QexMenuItemList( name );
	menuItemList.append( item );
	return item;

//	return NULL;
}

void	QexMenuList::makeMenuBar( QMenuBar *menuBar )
{
	if (menuBar == NULL)
		return;

	menuBar->clear();

	foreach( QexMenuItemList* i, menuItemList )
	{
		QList<QAction*> menuItems = i->getMenuItems();

		// dont generate a menu, if it contains no entries
		if (menuItems.count() == 0)
			continue;
		QMenu *m = new QMenu( i->getName(), menuBar->window() );

		// populate the new menu, with the desired actions
		foreach( QAction *a, menuItems )
		{
			m->addAction( a );
		}

		// add it to the menubar
		menuBar->addMenu( m );
	}
}

void	QexMenuList::installMenu( QexMenuList *m )
{
	foreach( QexMenuItemList* i, m->menuItemList )
	{
		foreach( QAction *a, i->getMenuItems() )
		{
			add( i->getName(), a );
		}
	}
}


void	QexMenuList::removeMenu( QexMenuList *m )
{
	foreach( QexMenuItemList* i, m->menuItemList )
	{
		foreach( QAction *a, i->getMenuItems() )
		{
			remove( a );
		}
	}
}


int	QexMenuList::count()
{
	return menuItemList.count();
}

void	QexMenuList::clear()
{
	menuItemList.clear();
}

bool	QexMenuList::empty()
{
	return menuItemList.empty();
}

QexMenuItemList* QexMenuList::operator[] ( const QString name )
{
	return getMenu( name );
}


/**
 * \class QITabInterface
 * \brief Inteface for displaying menus and toolbars on demand
 * 
 * This interface lets you the ability for changing the menus
 * and toolbars on the main window, according to the widget
 * on focus. This ability can be usefull on QTabWidget or 
 * any other MDI implementation. When the user selects a new tab
 * the menus will change to match the content of the widget on focus.
 * 
 * For example: 
 * You can add to the tab widget a widget for displaying text, 
 * a widget for editing images and a help browser. Each of those 
 * widgets need different toolbars.
 * 
 * You you need to do is derive your class from this interface
 * (we will call it from now "implement the interface"), implement
 * another interface on the main window (QITabWinInterface), and
 * use the QexTabWidget instead of the original QTabWidget supplied
 * by Trolltech.
 * 
 * On the constructor of the new class, you need to constuct the \p toolBar
 * property, and use the \p menus property to define the menus on your
 * widget.
 *
 * Here is a small example of how to use this interface: 
 * \code
 *	class QexEditor : public QTextEdit, public QITabInterface
 *	{
 *	public:
 *		QexEditor( QWidget *parent=0 );
 *	
 *	private:
 *		QAction *actionCopy;
 *		QAction *actionPaste;
 *		QAction *actionFind;
 *		QAction *actionOptions;
 *	};
 *
 *	QexEditor::QexEditor( QWidget *parent ):QTextEdit( parent )
 *	{
 *		actionCopy	= new QAction( tr("&Copy"), this );
 *		actionPaste	= new QAction( tr("&Paste"), this  );
 *		actionFind	= new QAction( tr("&Find"), this  );
 *		actionOptions	= new QAction( tr("&Options"), this  );
 *	
 *		// define toolbar for this widget
 *		toolbar = new QToolBar;
 *		toolbar->addAction( actionCopy );
 *		toolbar->addAction( actionPaste );
 *	
 *		// define the menus for this widget
 *		menus["&Edit"]->add( actionPaste );
 *		menus["&Edit"]->add( actionCopy );
 *		menus["&Search"]->add( actionFind );
 *		menus["&Configuration"]->add( actionOptions );
 *	}
 * \endcode
 */


/**
 * \brief Default constructor for QITabInterface
 * 
 * Builds the QITabInterface.
 */
QITabInterface::QITabInterface()
{
	toolbar = NULL;
};


/**
 * \brief Destructor for QITabInterface
 * 
 * Default destructor for the tab interace
 */
QITabInterface::~QITabInterface()
{
};


/**
 * \brief Called when the widget is about to be displayed on the screen
 * 
 * The task of this function is to allocate the needed entries
 * in the main window, and displaying the corresponding toolbar.
 * 
 * You can overload this function if you want to do anything else
 * when the tab is displayed.
 * 
 * \see QITabInterface::hideMe()
 */
void QITabInterface::showMe()
{ 
	applyMenu(); 
	showToolBar();
};


/**
 * \brief Called when the widget is about to be hidden from the screen
 * 
 * The task of this function is to deallocate the needed entries
 * in the main window, and hiding the corresponding toolbar.
 * 
 * You can overload this function if you want to do anything else
 * when the tab is hidden.
 * 
 * \see QITabInterface::showMe()
 */
void QITabInterface::hideMe()
{
	removeMenu(); 
	hideToolBar();
};


 /**
  * \brief Add the menus to the main window this widget belongs to
  * 
  * This function will allocate the corresponding menus
  * at the window which it belongs to. Note that in order for this
  * function to work, the top window must provide the QITabWinInterface
  * interface.
  * 
  * This function is called automatically when the widget is beeing hideen,
  * and there is no need to call it yourself.
  * 
  * \see QITabWinInterface
  * \see QexMenuList
  */
void QITabInterface::applyMenu()
{
	QMainWindow *window = getMainWindow();

	if (window == NULL)
		return;

	QITabWinInterface *iface = dynamic_cast<QITabWinInterface*>(window);
	if (iface != NULL )
		iface->applyMenu( &menus );
}


/**
  * \brief Removes the menus to the main window this widget belongs to
  * 
  * This function will deallocate the corresponding menus
  * at the window which it belongs to. Note that in order for this
  * function to work, the top window must provide the QITabWinInterface
  * interface.
  * 
  * This function is called automatically when the widget is beeing hideen,
  * and there is no need to call it yourself.
  * 
  * \see QITabWinInterface
  * \see QexMenuList
  */
void QITabInterface::removeMenu()
{
	QMainWindow *window = getMainWindow();

	if (window == NULL)
		return;

	QITabWinInterface *iface = dynamic_cast<QITabWinInterface*>(window);
	if (iface != NULL )
		iface->removeMenu( &menus );
}


/**
 * \brief Add the widget's toolbar to the main window this widget belongs to
 *
 * This function will add the widget's toolbar to the main window (if a 
 * toolbar has been allocated). Note that in order for this
 * function to work, the top window must provide the QITabWinInterface
 * interface.
 * 
 * This function is called automatically when the widget is beeing displayed,
 * and there is no need to call it yourself.
 * 
 * \see QITabWinInterface
 * \see QexMenuList
 */
void QITabInterface::showToolBar()
{
	// do nothing, if user did not define a toolbar for this widget
	if (toolbar == NULL)
		return;

	QMainWindow *window = getMainWindow();

	if (window != NULL)
	{
		window->addToolBar( toolbar );
		toolbar->show();
	}
}


/**
 * \brief Remove the widget's toolbar from the main window this widget belongs to
 *
 * This function will remove the widget's toolbar from the main window (if a 
 * toolbar has been allocated). Note that in order for this
 * function to work, the top window must provide the QITabWinInterface
 * interface.
 * 
 * This function is called automatically when the widget is beeing displayed,
 * and there is no need to call it yourself.
 * 
 * \see QITabWinInterface
 * \see QexMenuList
 */
void QITabInterface::hideToolBar()
{
	// do nothing, if user did not define a toolbar for this widget
	if (toolbar == NULL)
		return;

	QMainWindow *window = getMainWindow();

	if (window != NULL)
	{
		toolbar->hide();
		window->removeToolBar( toolbar );
	}
}


/**
 * \brief Get a pointer to the window this widget belongs to
 * \return A pointer to a QMainWindow object
 *
 * This function will return a pointer to the window this widget belongs to.
 * If this widget is not inside a QMainWindow (for example in a QDialog or orthers)
 * this function will return NULL.
 */
QMainWindow* QITabInterface::getMainWindow()
{
	QWidget *widget = dynamic_cast<QWidget*>(this);
	
	if (widget == NULL)
		return NULL;

	QMainWindow *window = dynamic_cast<QMainWindow*>(widget->window());

	return window;
}

/**
 * \class 
 *
 *
 */
QITabWinInterface::QITabWinInterface()
{
}

QITabWinInterface::~QITabWinInterface()
{
}

void QITabWinInterface::applyMenu( QexMenuList *newMenu )
{
	menus.installMenu( newMenu );

	QWidget     *widget = dynamic_cast<QWidget*>(this);
	
	if (widget == NULL)
		return;

	QMainWindow *window = dynamic_cast<QMainWindow*>(widget->window());
	if (window == NULL)
		return;

	menus.makeMenuBar( window->menuBar() );
}

void QITabWinInterface::removeMenu( QexMenuList *oldMenu )
{	
	menus.removeMenu( oldMenu );

	QWidget     *widget = dynamic_cast<QWidget*>(this);
	
	if (widget == NULL)
		return;

	QMainWindow *window = dynamic_cast<QMainWindow*>(widget->window());
	if (window == NULL)
		return;

	menus.makeMenuBar( window->menuBar() );
}

