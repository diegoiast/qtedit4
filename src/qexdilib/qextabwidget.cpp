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
	mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 0 );
	mainLayout->setMargin( 0 );
	this->setLayout( mainLayout );

	mainTab = new QTabWidget( this);
	mainLayout->addWidget( mainTab );

	openButton = new QToolButton( mainTab );
	openButton->setAutoRaise( true );
	openButton->setIcon( QIcon(":images/tab_new.png") );
	openButton->setToolTip( tr("New file") );
	mainTab->setCornerWidget( openButton, Qt::TopLeftCorner );

	closeButton = new QToolButton( mainTab );
	closeButton->setAutoRaise( true );
	closeButton->setIcon( QIcon(":images/tab_remove.png") );
	closeButton->setToolTip( tr("Close tab") );
	mainTab->setCornerWidget( closeButton, Qt::TopRightCorner );

	connect( mainTab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
	activeWidget = NULL;
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

	delete openButton;
	delete closeButton;
	delete mainTab;
	delete mainLayout;
}

int QexTabWidget::addTab( QWidget * child, const QString & label )
{
	int i = mainTab->addTab( child, label );
	
	// if more then one tab is available, the currentChanged() signal
	// will be emmited, when one, we need to hack
	if (mainTab->count() == 1)
	{
		activeWidget = child;
		QITabInterface *iface = dynamic_cast<QITabInterface*>(activeWidget);
		if (iface != NULL)
			iface->showMe();
		iface = NULL;
	}

	return i;
}

int QexTabWidget::addTab( QWidget * child, const QIcon & icon, const QString & label )
{
	int i = mainTab->addTab( child, icon, label );

	// if more then one tab is available, the currentChanged() signal
	// will be emmited, when one, we need to hack
	if (mainTab->count() == 1)
	{
		activeWidget = child;
		QITabInterface *iface = dynamic_cast<QITabInterface*>(activeWidget);
		if (iface != NULL)
			iface->showMe();
	}

	return i;
}


void QexTabWidget::removeTab ( int index )
{
	// this is ugly... why do i need to use C casts here?
	QWidget *deletedWidget = (QWidget*) widget( index );	
	QITabInterface *iface = dynamic_cast<QITabInterface*>(deletedWidget);

	if (iface != NULL)
		iface->hideMe();

	mainTab->removeTab( index );
}

const int QexTabWidget::currentIndex()
{
	return mainTab->currentIndex();
}

QWidget* QexTabWidget::currentWidget()
{
	return mainTab->currentWidget();
}

const QWidget * QexTabWidget::widget ( int index )
{
	return mainTab->widget( index );
}

const int QexTabWidget::count()
{
	return mainTab->count();
}

void QexTabWidget::setCurrentWidget ( QWidget * widget )
{
	mainTab->setCurrentWidget( widget );
}

void QexTabWidget::setCurrentIndex( int index )
{
	mainTab->setCurrentIndex( index );
}

void QexTabWidget::tabChanged( int i )
{
	QITabInterface *iface = NULL;
	QWidget *w = mainTab->widget( i );

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
