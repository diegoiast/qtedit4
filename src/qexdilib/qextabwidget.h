#ifndef __QEX_TAB_WIDGET_H__
#define __QEX_TAB_WIDGET_H__

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

class QexTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	QexTabWidget();
	~QexTabWidget();
	
public slots:
	void tabChanged( int i );

protected:
	virtual void tabInserted ( int index );
	virtual void tabRemoved ( int index );

private:
	QWidget *activeWidget;
};



#endif // __QEX_TAB_WIDGET_H__
