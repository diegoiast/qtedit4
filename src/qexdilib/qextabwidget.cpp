#include <QStackedWidget>

#include "qextabwidget.h"
#include "qexitabwidget.h"


/**
 * \brief Default constructor
 * Builds the tab widget.
 * 
 * QexTabWidget is an advanced tab widget, which knows about the 
 * widgets contained inside, and if the widget implement the QITabInterface
 * interface, it will query them for extra functions: one to generate toolbars and 
 * menu entries needed, and another to remove those entries.
 * 
 * If you want to use the full features
 */
QexTabWidget::QexTabWidget()
{
	connect( this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
	activeWidget = NULL;

	// this ugly trick has been stollen from "assistant"
	QStackedWidget *stack = qFindChild<QStackedWidget*>(this);
	Q_ASSERT(stack);
	stack->setContentsMargins(0, 0, 0, 0);
}

/**
 * \brief Default destructor
 * Destroys the QexTabWidget.
 * 
 * Among all the other tasks needed to gracefully close this 
 * widget, will also ask the active widget to un apply all the 
 * changes done (menus, toolbars, etc)
 */
QexTabWidget::~QexTabWidget()
{
	if (activeWidget)
	{
		QITabInterface *iface = dynamic_cast<QITabInterface*>(activeWidget);
		if (iface != NULL)
			iface->hideMe();
		activeWidget = NULL;
	}
}

void QexTabWidget::tabInserted ( int index )
{
	QWidget *deletedWidget = (QWidget*) widget( index );	
	QITabInterface *iface = dynamic_cast<QITabInterface*>(deletedWidget);

	if (iface != NULL)
		iface->showMe();
}

void QexTabWidget::tabRemoved ( int index )
{
	// this is ugly... why do i need to use C casts here?
	QWidget *deletedWidget = (QWidget*) widget( index );	
	QITabInterface *iface = dynamic_cast<QITabInterface*>(deletedWidget);

	if (iface != NULL)
		iface->hideMe();

}

void QexTabWidget::tabChanged( int i )
{
	QITabInterface *iface = NULL;
	QWidget *w = widget( i );

	// nothing to do, if the same tab has been selected twise
	if ( w == activeWidget)
		return;
	
	if (activeWidget)
	{
		iface = dynamic_cast<QITabInterface*>(activeWidget);
		if (iface != NULL)
			iface->hideMe();
	}

	activeWidget = w;

	if (activeWidget)
	{
		iface = dynamic_cast<QITabInterface*>(activeWidget);
		if (iface != NULL)
			iface->showMe();
	}
}
