#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
	void closeEvent(QCloseEvent *event);
// 	void contextMenuEvent(QContextMenuEvent *event);

public slots:
	void selectNextTab();
	void selectPrevTab();
	void selectEditor();
	void editorChanged();
	void editorCursorPositionChanged();
	
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSave( QTextEdit *e );
	bool fileSaveAs();
	bool fileSaveAs( QTextEdit *e );
	void fileClose();

	void editUndo();
	void editRedo();
	void editCopy();
	void editCut();
	void editPaste();
	
	void searchFind();
	void searchFindNext();
	void searchFindPrev();
	void searchReplace();
	void searchGotoLine();
	
	void optionsConfiguration();
	void about();

private:
	void createActions();
	void createMenus();
	void createToolbar();
	void saveStatus();
	void loadStatus();
	QString MainWindow::getFileName( QString fileName );
	void loadFile( QString fileName );
	QTextEdit* getCurrentEditor();
	bool canCloseEditor( QTextEdit *e );
	
	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *searchMenu;
	QMenu *optionsMenu;
	QMenu *helpMenu;
	QToolBar *fileToolBar;

	// internal actions
	QAction *actionNextTab;
	QAction *actionPreviousTab;
	QAction *actionSelectEditor;

	// menus and toolbars - file
	QAction *actionNew;
	QAction *actionOpen;
	QAction *actionSave;
	QAction *actionSaveAs;
	QAction *actionClose;
	QAction *actionExit;
	// menus and toolbars - edit
	QAction *actionUndo;
	QAction *actionRedo;
	QAction *actionCut;
	QAction *actionCopy;
	QAction *actionPaste;
	// menus and toolbars - options
	QAction *actionConfig;
	// menus and toolbars - search
	QAction *actionFind;
	QAction *actionReplace;
	QAction *actionFindNext;
	QAction *actionFindPrev;
	QAction *actionGotoLine;
	// menus and toolbars - about
	QAction *actionAbout;
	QAction *actionAboutQt;
	
	QTextEdit *currentEditor;
	OptionsDialog *optionsDialog;
	QTabWidget *mainTab;
	kateItemDataManager defColors;
};

#endif
