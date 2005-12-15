#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>

#include "qexdilib/qexitabwidget.h"
#include "qexdilib/qextabwidget.h"

#include "kateitemdatamanager.h"
#include "optionsdialog.h"

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

class MainWindow : public QMainWindow, public QITabWinInterface
{
    Q_OBJECT

public:
    MainWindow();

protected:
	void closeEvent(QCloseEvent *event);
// 	void contextMenuEvent(QContextMenuEvent *event);

public slots:
	void selectEditor();
	void editorCursorPositionChanged();
	
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSave( QTextEdit *e );
	bool fileSaveAs();
	bool fileSaveAs( QTextEdit *e );
	void fileClose();
	
	void optionsConfiguration();
	void about();

private:
	void createActions();
	void createMenus();
	void createToolbar();
	void saveStatus();
	void loadStatus();
	QString getFileName( QString fileName );
	void loadFile( QString fileName );
	QTextEdit* getCurrentEditor();
	bool canCloseEditor( QTextEdit *e );
	
	QToolBar *fileToolBar;

	// internal actions
	QAction *actionSelectEditor;

	// menus and toolbars - file
	QAction *actionNew;
	QAction *actionOpen;
	QAction *actionClose;
	QAction *actionExit;
	// menus and toolbars - options
	QAction *actionConfig;
	// menus and toolbars - about
	QAction *actionAbout;
	QAction *actionAboutQt;
	
	OptionsDialog *optionsDialog;
// 	QTabWidget *mainTab;
	QexTabWidget *mainTab;
	kateItemDataManager defColors;
};

#endif
