#include "Qtedit4MainWindow.h"
#include "qmditabwidget.h"

#include <QTabWidget>
#include <QWidget>
#include <QToolButton>
#include <QBoxLayout>
#include <QTextEdit>
#include <QMenuBar>

#include <QDebug>

// Qtedit4MainWindow::Qtedit4MainWindow( QWidget * parent, Qt::WindowFlags flags )
// 	: PluginManager( parent, flags )
Qtedit4MainWindow::Qtedit4MainWindow()
	: PluginManager()
{
	// http://wiki.qtcentre.org/index.php?title=Embedded_resources
	QIcon		closeIcon(":/trolltech/styles/commonstyle/images/standardbutton-close-16.png");
	QIcon		maxIcon(":/trolltech/styles/commonstyle/images/up-16.png");
	QIcon		minIcon(":/trolltech/styles/commonstyle/images/down-16.png");
	
	QWidget		*rightButtons = new QWidget;
	QBoxLayout	*l = new QBoxLayout(QBoxLayout::LeftToRight);
	QToolButton	*minButton = new QToolButton;
	QToolButton	*maxButton = new QToolButton;
	QToolButton	*closeButton = new QToolButton;
	
	closeButton->setAutoRaise( true );
	closeButton->setIcon( closeIcon );
	closeButton->setFixedSize(QSize(16,16));
	connect( closeButton, SIGNAL(clicked()), this, SLOT(on_closeButton_clicked()));
	
	minButton->setAutoRaise( true );
	minButton->setIcon( minIcon );
	minButton->setFixedSize(QSize(16,16));
	connect( minButton, SIGNAL(clicked()), this, SLOT(on_minimizeButton_clicked()));
	
	maxButton->setAutoRaise( true );
	maxButton->setIcon( maxIcon );
	maxButton->setFixedSize(QSize(16,16));
	connect( maxButton, SIGNAL(clicked()), this, SLOT(on_maximizeButton_clicked()));
	
	l->addWidget( minButton );
	l->addWidget( maxButton );
	l->addWidget( closeButton );
	rightButtons->setLayout( l );
	tabWidget->setCornerWidget( rightButtons, Qt::TopRightCorner );

	Qt::WindowFlags flags = windowFlags();
	if (isMaximized())
		flags |=  Qt::FramelessWindowHint;
	else
		flags &= ~Qt::FramelessWindowHint;
	
	setWindowFlags( flags );
	menuBar()->hide();
}

void Qtedit4MainWindow::resizeEvent( QResizeEvent * event )
{
	Qt::WindowFlags flags = windowFlags();
	
	if (isMaximized())
	{
		flags |=  Qt::FramelessWindowHint;
		setWindowFlags( flags );
		showMaximized();
	}
	else
	{
		flags &= ~Qt::FramelessWindowHint;
		setWindowFlags( flags );
		show();
	}
	
	QMainWindow::resizeEvent( event );
}


void Qtedit4MainWindow::on_maximizeButton_clicked()
{
	if (isMaximized())
		showNormal();
	else
		showMaximized();
}

void Qtedit4MainWindow::on_minimizeButton_clicked()
{
	showMinimized();
}

void Qtedit4MainWindow::on_closeButton_clicked()
{
	close();
}
