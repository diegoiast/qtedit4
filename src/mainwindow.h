#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>

#include "qmdilib/qmdihost.h"

#include "kateitemdatamanager.h"
#include "configdialog.h"

/**
 * \file    mainwindow.h
 * \brief   Declaration of the main window class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */
 
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QString;

class MainWindow : public QMainWindow, public qmdiHost
{
    Q_OBJECT

public:
    MainWindow();

protected:
	void closeEvent(QCloseEvent *event);
// 	void contextMenuEvent(QContextMenuEvent *event);

public slots:
	void fileNew();
	void fileOpen();
	void fileClose();
	
	void optionsConfiguration();
	void helpBrowseQtDocs();
	void helpBrowseQtEditDocs();
	void about();

private:
	void createActions();
	void createMenus();
	void createToolbar();
	void saveStatus();
	void loadStatus();
	QString getFileName( QString fileName );
	void loadFile( QString fileName );
	
	QAction *actionNew;
	QAction *actionOpen;
	QAction *actionClose;
	QAction *actionExit;
	QAction *actionConfig;
	QAction *actionBrowseQtDocs;
	QAction *actionBrowseQtEditDocs;
	QAction *actionAbout;
	QAction *actionAboutQt;
	
        ConfigurationDialog *configDialog;
	QTabWidget *mainTab;
	kateItemDataManager defColors;
};

#endif
