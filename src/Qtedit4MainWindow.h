#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

// #include <PluginManager>
#include <pluginmanager.h>
#include <QResizeEvent>
#include <QIcon>

class QToolButton;
class QWidget;

class Qtedit4MainWindow : public PluginManager
{
	Q_OBJECT
public:
// 	Qtedit4MainWindow( QWidget * parent = 0, Qt::WindowFlags flags = 0 );
	Qtedit4MainWindow();
	
public slots:
	void on_maximizeButton_clicked();
	void on_minimizeButton_clicked();
	void on_closeButton_clicked();

protected:
	void resizeEvent( QResizeEvent * event );

private:
	QIcon	m_closeIcon;
	QIcon	m_maxIcon;
	QIcon	m_minIcon;
	
	QWidget *m_rightButtons;
	QToolButton	*m_minButton;
	QToolButton	*m_maxButton;
	QToolButton	*m_closeButton;
};

#endif // MYMAINWINDOW_H
