#include <QtGui>
#include <QtDebug>

#include "qecpphighlighter.h"
#include "qecodeeditor.h"
#include "mainwindow.h"
#include "textdisplay.h"


/**
 * \file    mainwindow.cpp
 * \brief   Implementation of the main window class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 * \
 */


/**
 * \class MainWindow
 * \brief The main window of the application
 * 
 * This is the main window seen on screen. When this window
 * is closed, this means the application has been terminated.
 * 
 * 
 */


/**
 * \brief Constructor for the main window
 * 
 * This constructs a main window, and generates the actions, menus
 * and toolbars needed.
 * 
 * \todo This class also loads the default highlight mode,
 *       shuold be move to another class?
 * \todo Do we need to modify the title of the main window?
 */
MainWindow::MainWindow()
{
	defColors.load( "data/default/kate.xml" );
	mainTab = new QTabWidget( this );
	currentEditor = NULL;
	connect( mainTab, SIGNAL(currentChanged(int)), this, SLOT(editorChanged()) );
	setCentralWidget( mainTab );
	
	createActions();
	createMenus();
	createToolbar();
	
	// some internal hacks
	addAction( actionPreviousTab );
	addAction( actionNextTab );
	addAction( actionSelectEditor );
	
	setWindowTitle( tr("QtEdit 4 [*]") );
	statusBar()->showMessage( tr("Welcome"), 5000 );
	loadStatus();

	optionsDialog = new OptionsDialog( this );
	optionsDialog->hide();
}


/**
 * \brief Called when the window is been closed
 *
 * This function is called when the main window is about to 
 * be closed. We use this to save the status of the application.
 * 
 * \warning This function changes the value of the event passed
 *          because otherwise, gcc complains and issues a warning.
 */
void MainWindow::closeEvent( QCloseEvent *event )
{
	saveStatus();

	// ugly hack to shut up warnings from gcc...
	event = 0;
}

/*
NOT USED, left as reference
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(actionCut);
    menu.addAction(actionCopy);
    menu.addAction(actionPaste);
    menu.exec(event->globalPos());
}
*/


/**
 * \brief move the focus to the next tab (SLOT)
 *
 * This slot is called when the user presses the ALT+right
 * keyboard combination (alt+left on RTL desktops).
 * This function does the job of selecting the needed
 * tab number.
 * 
 * \see selectPrevTab
 */
void MainWindow::selectNextTab()
{
	int i = mainTab->currentIndex();
	++i;

	if ( i >= mainTab->count() )
		i = 0;
		
	mainTab->setCurrentIndex( i );
}


/**
 * \brief move the focus to the previous tab (SLOT)
 *
 * This slot is called when the user presses the ALT+left
 * keyboard combination (alt+right on RTL desktops)
 * This function does the job of selecting the needed
 * tab number.
 * 
 * \see selectNextTab
 */
void MainWindow::selectPrevTab()
{
	int i = mainTab->currentIndex();
	--i;

	if ( i < 0 )
		i = mainTab->count() - 1;
	
	mainTab->setCurrentIndex( i );
}


/**
 * \brief set the focus on the editor (SLOT)
 *
 * This function restores the focus to the current editor,
 * to the editor in the mainTab.
 */
void MainWindow::selectEditor()
{
	QWidget *w = getCurrentEditor();
	
	if (w)
		w->setFocus();
}


void MainWindow::editorChanged()
{
	// disconnect old connections
	if (currentEditor)
	{
		disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)), actionSave, SLOT(setEnabled(bool)));
		disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
		disconnect(currentEditor->document(), SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
		disconnect(currentEditor->document(), SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));

		disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
		disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
	}

	// make the new ones
	currentEditor = getCurrentEditor();

	if (!currentEditor)
	{
		actionSave->setEnabled( false );
		actionUndo->setEnabled( false );
		actionRedo->setEnabled( false );
		actionCut->setEnabled( false );
		actionCopy->setEnabled( false );
		
		return;
	}	
	
	const bool selection = currentEditor->textCursor().hasSelection();
// 	TextDisplay *textDisplay = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	
	setWindowModified( currentEditor->document()->isModified() );
	actionSave->setEnabled( currentEditor->document()->isModified() );
	actionUndo->setEnabled( currentEditor->document()->isUndoAvailable() );
	actionRedo->setEnabled( currentEditor->document()->isRedoAvailable() );
		
	connect(currentEditor->document(), SIGNAL(modificationChanged(bool)), actionSave, SLOT(setEnabled(bool)));
	connect(currentEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
	connect(currentEditor->document(), SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
	connect(currentEditor->document(), SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));
		
	connect(currentEditor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
	connect(currentEditor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

	
	actionCut->setEnabled(selection);
	actionCopy->setEnabled(selection);
}


void MainWindow::editorCursorPositionChanged()
{
	QPoint p = qobject_cast<TextDisplay*>(mainTab->currentWidget())->getCursorLocation();
	
	statusBar()->showMessage( QString("line: %1, culumn %2").arg(p.y()).arg(p.x() ) );
}


void MainWindow::fileNew()
{
	loadFile( "" );
}

void MainWindow::fileOpen()
{
	QString s = QFileDialog::getOpenFileName(
		this,
		"Choose a file to open",
		"",
		"C/C++ cources (*.c *.c++ *.cpp *.h *.moc);;"
		"C/C++ headers (*.h *.hxx);;"
		"Qt Project files (*.pro *.pri);;"
		"All files (*.*)"
	);
	
	if ( s.isEmpty() )
		return;

	loadFile( s );
}

bool MainWindow::fileSave()
{
	return fileSave( (QECodeEditor*) getCurrentEditor() );
}

bool MainWindow::fileSave( QTextEdit *edit )
{
	if (!edit)
		return false;

	QECodeEditor *e = qobject_cast<QECodeEditor*>(edit);
	if (!e)
		return fileSaveAs();
		
	QString fileName = e->getFileName();

	if (fileName.isEmpty())
		return fileSaveAs();
	
	e->saveFile( fileName );
	setWindowModified( false );
	
	return true;
}

bool MainWindow::fileSaveAs()
{
	return fileSaveAs( (QECodeEditor*) getCurrentEditor() );
}

bool MainWindow::fileSaveAs( QTextEdit *edit )
{
	if (!edit)
		return false;
		
	QECodeEditor *e = qobject_cast<QECodeEditor*>(edit);
	if (!e)
		return false;

	
	QString s = QFileDialog::getSaveFileName(
		this,
		"Choose a file to save",
		"",
		"C/C++ cources (*.c *.c++ *.cpp *.h *.moc);;"
		"C/C++ headers (*.h *.hxx);;"
		"Qt Project files (*.pro *.pri);;"
		"All files (*.*)"
	);
	
	if ( s.isEmpty() )
		return false;

	e->saveFile( s );
	setWindowModified( false );

	return true;
}

void MainWindow::fileClose()
{
	if (!canCloseEditor(getCurrentEditor()) )
		return;
		
	QWidget *w = mainTab->currentWidget();
	mainTab->removeTab( mainTab->currentIndex() );

	// this is wierd, I was under the impression this
	// signal will be emmited for me...
	// lets do it mannually, otherwise when we open a new
	// tab after the last one has been close, it will try to
	// disconnect a nont existant tab. A new tab will be selected
	// anyway (if such one exists), therefor I can set the currentEditor to null
	// Note that this code is done also when selecting a new tav.
	// see also editorChanged()
	if (currentEditor)
	{
		disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)), actionSave, SLOT(setEnabled(bool)));
		disconnect(currentEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
		disconnect(currentEditor->document(), SIGNAL(undoAvailable(bool)), actionUndo, SLOT(setEnabled(bool)));
		disconnect(currentEditor->document(), SIGNAL(redoAvailable(bool)), actionRedo, SLOT(setEnabled(bool)));
	
		disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
		disconnect(currentEditor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));
		currentEditor = NULL;
	}
	delete w;

}


void MainWindow::editUndo()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->undo();
}

void MainWindow::editRedo()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->redo();
}

void MainWindow::editCopy()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->copy();
}

void MainWindow::editCut()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->cut();

}

void MainWindow::editPaste()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->paste();
}

void MainWindow::searchFind()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->find();
}

void MainWindow::searchFindNext()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->findNext();
}

void MainWindow::searchFindPrev()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->findPrev();
}

void MainWindow::searchReplace()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->replace();
}

void MainWindow::searchGotoLine()
{
	TextDisplay *t = qobject_cast<TextDisplay*>(mainTab->currentWidget());
	if (!t)
		return;

	t->gotoLine();
}


void MainWindow::optionsConfiguration()
{
	optionsDialog->show();
}

void MainWindow::about()
{
	QMessageBox::about( this, tr("About QtEdit4"),
		tr("The 4rth version of my Qt editor.")
	);
}



void MainWindow::createActions()
{
	actionNextTab = new QAction( tr("&Next tab"), this);
	actionNextTab->setShortcut( tr("Alt+Right") );
	connect( actionNextTab, SIGNAL(triggered()), this, SLOT(selectNextTab()) );
	
	actionPreviousTab = new QAction( tr("&Previous tab"), this);
	actionPreviousTab->setShortcut( tr("Alt+Left") );
	connect( actionPreviousTab, SIGNAL(triggered()), this, SLOT(selectPrevTab()) );

	actionSelectEditor = new QAction( tr("Select editor"), this);
	actionSelectEditor->setShortcut( tr("Esc") );
	connect( actionSelectEditor, SIGNAL(triggered()), this, SLOT(selectEditor()) );
	
	// file menu
	actionNew = new QAction( QIcon(":images/filenew.png"), tr("&New"), this);
	actionNew->setShortcut(tr("Ctrl+N"));
	actionNew->setStatusTip(tr("Create a new file"));
	connect( actionNew, SIGNAL(triggered()), this, SLOT(fileNew()) );
	
	actionOpen = new QAction( QIcon(":images/fileopen.png"), tr("&Open..."), this);
	actionOpen->setShortcut(tr("Ctrl+O"));
	actionOpen->setStatusTip(tr("Open an existing file"));
	connect( actionOpen, SIGNAL(triggered()), this, SLOT(fileOpen()) );
	
	actionSave = new QAction( QIcon(":images/filesave.png"), tr("&Save"), this);
	actionSave->setShortcut(tr("Ctrl+S"));
	actionSave->setStatusTip(tr("Save the document to disk"));
	actionSave->setEnabled( false );
	connect( actionSave, SIGNAL(triggered()), this, SLOT(fileSave()) );
	
	actionSaveAs = new QAction( QIcon(":images/filesaveas.png"), tr("&Save as"), this);
	actionSaveAs->setStatusTip(tr("Save the document to disk"));
	connect( actionSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAs()) );
	
	actionClose = new QAction( QIcon(":images/fileclose.png"), tr("C&lose"), this);
	actionClose->setShortcut(tr("Ctrl+W"));
	actionClose->setStatusTip(tr("Close the active tab"));
	connect( actionClose, SIGNAL(triggered()), this, SLOT(fileClose()) );
	
	actionExit = new QAction( QIcon(":images/exit.png"), tr("E&xit"), this);
	actionExit->setShortcut(tr("Ctrl+Q"));
	actionExit->setStatusTip(tr("Exit the application"));
	connect( actionExit, SIGNAL(triggered()), this, SLOT(close()) );

	// edit menu
	actionUndo = new QAction( QIcon(":images/undo.png"), tr("&Undo"), this);
	actionUndo->setShortcut(tr("Ctrl+Z"));
	actionUndo->setStatusTip(tr("Undo the last operation"));
	connect( actionUndo, SIGNAL(triggered()), this, SLOT(editUndo()) );
	
	actionRedo = new QAction( QIcon(":images/redo.png"), tr("&Redo"), this);
	actionRedo->setShortcut(tr("Ctrl+Y"));
	actionRedo->setStatusTip(tr("Redo the last operation"));
	connect( actionRedo, SIGNAL(triggered()), this, SLOT(editRedo()) );
	
	actionCut = new QAction( QIcon(":images/editcut.png"), tr("Cu&t"), this);
	actionCut->setShortcut(tr("Ctrl+X"));
	actionCut->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	connect( actionCut, SIGNAL(triggered()), this, SLOT(editCut()) );
	
	actionCopy = new QAction( QIcon(":images/editcopy.png"), tr("&Copy"), this);
	actionCopy->setShortcut(tr("Ctrl+C"));
	actionCopy->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	connect( actionCopy, SIGNAL(triggered()), this, SLOT(editCopy()) );
	
	actionPaste = new QAction( QIcon(":images/editpaste.png"), tr("&Paste"), this);
	actionPaste->setShortcut(tr("Ctrl+V"));
	actionPaste->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	actionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
	connect( actionPaste, SIGNAL(triggered()), this, SLOT(editPaste()) );

	// search menu
	actionFind = new QAction( QIcon(":images/find.png"), tr("&Find"), this);
	actionFind->setShortcut(tr("Ctrl+F"));
	actionFind->setStatusTip(tr("Search for a string in the currently opened window"));
	connect( actionFind, SIGNAL(triggered()), this, SLOT(searchFind()) );
	
	actionReplace = new QAction( QIcon(":images/find.png"), tr("&Replace"), this);
	actionReplace->setShortcut(tr("Ctrl+R"));
	actionReplace->setStatusTip(tr("Replace a string in the currently opened window"));
	connect( actionReplace, SIGNAL(triggered()), this, SLOT(searchReplace()) );
	
	actionGotoLine = new QAction( QIcon(":images/find.png"), tr("&Goto line"), this);
	actionGotoLine ->setShortcut(tr("Ctrl+G"));
	actionGotoLine ->setStatusTip(tr("Scroll to a new line in the currently opened window"));
	connect( actionGotoLine, SIGNAL(triggered()), this, SLOT(searchGotoLine()) );
	
	actionFindNext = new QAction( QIcon(":images/next.png"), tr("&Find next"), this);
	actionFindNext->setShortcut(tr("F3"));
	actionFindNext->setStatusTip(tr("Search for the next match of a string the currently opened window"));
	connect( actionFindNext, SIGNAL(triggered()), this, SLOT(searchFindNext()) );

	actionFindPrev = new QAction( QIcon(":images/previous.png"), tr("&Find previous"), this);
	actionFindPrev->setShortcut(tr("Shift+F3"));
	actionFindPrev->setStatusTip(tr("Search for the previous match of a string the currently opened window"));
	connect( actionFindPrev, SIGNAL(triggered()), this, SLOT(searchFindPrev()) );

	// options menu
	actionConfig = new QAction( tr("Configuration"), this  );
	actionConfig->setStatusTip("Change the default behaviour of the editor");
	connect( actionConfig, SIGNAL(triggered()), this, SLOT(optionsConfiguration()) );
	
	// help menu
	actionAbout = new QAction(tr("&About"), this);
	actionAbout->setStatusTip(tr("Show the application's About box"));
	connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	
	actionAboutQt = new QAction(tr("About &Qt"), this);
	actionAboutQt->setStatusTip(tr("Show the Qt library's About box"));
	connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(actionNew);
	fileMenu->addAction(actionOpen);
	fileMenu->addAction(actionSave);
	fileMenu->addAction(actionSaveAs);
	fileMenu->addSeparator();
	fileMenu->addAction(actionClose);
	fileMenu->addSeparator();
	fileMenu->addAction(actionExit);
	
	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(actionUndo);
	editMenu->addAction(actionRedo);
	editMenu->addSeparator();
	editMenu->addAction(actionCut);
	editMenu->addAction(actionCopy);
	editMenu->addAction(actionPaste);
	
	searchMenu = menuBar()->addMenu(tr("&Search"));
	searchMenu->addAction( actionFind );
	searchMenu->addAction( actionFindNext );
	searchMenu->addAction( actionFindPrev );
	searchMenu->addAction( actionReplace );
	searchMenu->addSeparator();
	searchMenu->addAction( actionGotoLine );

	optionsMenu = menuBar()->addMenu( tr("&Options") );
	optionsMenu->addAction( actionConfig );
	
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(actionAbout);
	helpMenu->addAction(actionAboutQt);
}

void MainWindow::createToolbar()
{
	fileToolBar = addToolBar( tr("File") );
	fileToolBar->setObjectName( "File" );
	fileToolBar->addAction(actionNew);
	fileToolBar->addAction(actionOpen);
	fileToolBar->addAction(actionSave);
	fileToolBar->addAction(actionClose);
}

QString MainWindow::getFileName( QString fileName )
{
	int i = fileName.lastIndexOf( "/" );

        // if not found, try windows path
	if ( i == -1 )
		i = fileName.lastIndexOf( "\\" );

	if (i > 0 )
		fileName = fileName.remove( 0, i+1 );

	return fileName;
}

void MainWindow::loadFile( QString fileName )
{
	QECPPHighlighter *h = new QECPPHighlighter( &defColors );
	QECodeEditor *edit = new QECodeEditor( NULL, h );
	QString tabName;

	if (!fileName.isEmpty())
	{
		edit->loadFile( fileName );
		tabName = getFileName( fileName );
	}
	else
		tabName = tr("NONAME");

	TextDisplay *t = new TextDisplay( edit );
	int i = mainTab->addTab( t, tabName );
	mainTab->setCurrentWidget( t );
	mainTab->setCurrentIndex( i );
	edit->setFocus();
	
	if ( !fileName.isEmpty() )
		statusBar()->showMessage( tr("File \"%1\" loaded").arg(fileName), 5000 );
}

QTextEdit* MainWindow::getCurrentEditor()
{
	QWidget *w = mainTab->currentWidget();

	if (!w)
		return NULL;

	if (!w->inherits("TextDisplay"))
		return NULL;

	w = qobject_cast<TextDisplay*>(w)->getEditor();

	if (!w)
		return NULL;
	else
		if (w->inherits("QTextEdit") )
			return (QTextEdit*)w;
		else
			return NULL;
}


void MainWindow::saveStatus()
{
	QSettings settings;
	QString openFiles = "";
	
	settings.setValue( "main/state", saveState(0) );
	settings.setValue( "main/position", pos() );
	settings.setValue( "main/size", size() );
	
	for (int i=0; i<mainTab->count(); i++)
	{
		if (mainTab->widget(i)->inherits( "TextDisplay" ) )
		{
			// TODO what if the widget is not a TextDisplay?
			// TODO how about using qobject_cast instead of C cast?
			TextDisplay* e = (TextDisplay*)mainTab->widget(i);
			openFiles += //e->getEditor()->getFileName() + ";";
			qobject_cast<QECodeEditor*>(e->getEditor())->getFileName() + ";";
		}
	}
	openFiles.remove( openFiles.size()-1, 1 );
 	settings.setValue( "editor/files", openFiles );
}

void MainWindow::loadStatus()
{
	QSettings settings;
	
	move( settings.value("main/position", QPoint(200, 200)).toPoint() );
	resize( settings.value("main/size", QSize(400, 400)).toSize() );
	restoreState( settings.value("main/state").toByteArray() );

	QStringList sl = settings.value("editor/files").toString().split( ';' );
	for ( int i=0; i<sl.count(); i++)
		if (! sl[i].isEmpty() )
			loadFile( sl[i] );
}

bool MainWindow::canCloseEditor( QTextEdit *e )
{
	if (!e)
		return true;
		
	if ( !e->document()->isModified() )
		return true;

	switch(QMessageBox::information(this, "QtEdit4",
		tr("The document contains unsaved changes\n"
		"Do you want to save the changes before exiting?" ),
		tr("&Save"), tr("&Don't save"), tr("Cancel"),
		0,    // Enter == button 0
		2)
		)  // Escape == button 2
	{
		case 0: // Save clicked or Alt+S pressed or Enter pressed.
			return fileSave();
			break;

		case 1: // Discard clicked or Alt+D pressed don't save but exit
			return true;

		case 2: // Cancel clicked or Escape pressed, don't exit
			return false;
	}

	return false;
}
