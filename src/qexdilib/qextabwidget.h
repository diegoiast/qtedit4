#ifndef __QEX_TAB_WIDGET_H__
#define __QEX_TAB_WIDGET_H__

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

class QexTabWidget : public QWidget
{
	Q_OBJECT
public:
	QexTabWidget();
	~QexTabWidget();
	
	int addTab( QWidget * child, const QString & label );
	int addTab( QWidget * child, const QIcon & icon, const QString & label );
	void removeTab ( int index );	
	const int currentIndex();
	QWidget* currentWidget();
	const QWidget * widget ( int index );
	const int count();

public slots:
	void setCurrentWidget ( QWidget * widget );
	void setCurrentIndex( int index );
	void tabChanged( int i );

private:
	QVBoxLayout *mainLayout;
	QTabWidget *mainTab;
	QWidget *activeWidget;	
	QToolButton *openButton, *closeButton;
};



#endif // __QEX_TAB_WIDGET_H__
