#ifndef __QI_TAB_WIDGET_H__
#define __QI_TAB_WIDGET_H__

#include <QList>
#include <QString>
#include <QToolBar>

class QMenuBar;
class QAction;
class QMainWindow;

class QexMenuItemList
{
public:
	QexMenuItemList( QString n );
	~QexMenuItemList();
	void	add( QAction *action );
	void	addSeparator();
	void	remove( QAction *action );
	bool	contains( QAction *action );
	int 	count();
	QString getName() { return name; };
	QList<QAction*> getMenuItems() { return menus; };

private:
	QList<QAction*> menus;
	QString name;

};

class QexMenuList
{
public:
	QexMenuList();
	~QexMenuList();
	
	void	add( QString menuName, QAction *action );
	void	remove( QAction *action );
	bool	contains( QAction *action );
	QexMenuItemList* getMenu( QString name );	
	void	makeMenuBar( QMenuBar *menuBar );

	void	installMenu( QexMenuList *m );
	void	removeMenu( QexMenuList *m );

	int	count();
	void	clear();
	bool	empty();

	QexMenuItemList* operator[] ( const QString name );

private:
	QList< QexMenuItemList* > menuItemList;
};


class QITabInterface
{
public:
	QITabInterface();
	virtual ~QITabInterface();

	virtual void showMe();
	virtual void hideMe();

	void applyMenu();
	void removeMenu();
	void showToolBar();
	void hideToolBar();
	
	QMainWindow* getMainWindow();

protected:
	QexMenuList menus;
	QToolBar *toolbar;
};

class QITabWinInterface
{
public:
	QITabWinInterface();
	virtual ~QITabWinInterface();

	virtual void applyMenu( QexMenuList *newMenu );
	virtual void removeMenu( QexMenuList *oldMenu );
	
protected:
	QexMenuList menus;
};

#endif // __QI_TAB_WIDGET_H__
