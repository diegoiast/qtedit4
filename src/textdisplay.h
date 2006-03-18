#ifndef __TEXT_DISPLAY_H__
#define __TEXT_DISPLAY_H__

#include <QTextDocument>
#include "qmdilib/qmdiclient.h"

class QTextEdit;
class QAction;
class QSplitter;
class QVBoxLayout;
class QTextBrowser;
class QSyntaxHighlighter;

class kateItemDataManager;
class QeGtkSourceViewLangDef;
class QeGTK_Highlighter;
class LineNumberWidget;


#include "ui_inlinefind.h"
#include "ui_inlinereplace.h"
#include "ui_inlinegotoline.h"

/**
 * \file    textdisplaycpp
 * \brief   Declaration of the textdisplay class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

class TextDisplay: public QWidget, public qmdiClient
{
Q_OBJECT
friend class MainWindow;
public:
	TextDisplay( QTextEdit *editor, QWidget *parent=NULL, kateItemDataManager *colors=NULL, bool ignoreConf=false );
	~TextDisplay();

	QTextEdit*	getEditor() {return editor; };
	void    	setEditor( QTextEdit *e, bool ignoreConf=false, bool isGotoLineEnabled=true );
	QWidget*	getInternalFind() { return internalFind; };
	QTextDocument*	document();

	void setColorDef( kateItemDataManager *newColors );
	kateItemDataManager* getColorDef() { return defColors; };

protected:
	void	createActions();
	
public slots:
	void	hideInternalWidgets();
	void	undo();
	void	redo();
	void	copy();
	void	cut();
	void	paste();
	void	find();
	void	replace();
	void	gotoLine();
	void	findNext();
	void	findPrev();
	void	incrementalSearch( QString s );
	void	findString( QString s, bool wholeDocument=false, QTextDocument::FindFlags findOptions=0 );
	void	doReplace();
	void	gotoLine( int i );

	QPoint	getCursorLocation();
	void	setCursorLocation( QPoint p );
	void	setCursorLocation( int x, int y );
	uint	getLineCount();
	
	bool	loadFile( QString fileName );

	bool	getGotoLineEnabled(){ return gotoLineEnabled; };
	void	setGotoLineEnabled( bool enabled );

	void	loadingTimer();
	void	updateConfiguration();

protected:
	QAction *actionUndo;
	QAction *actionRedo;
	QAction *actionCut;
	QAction *actionCopy;
	QAction *actionPaste;
	QAction *actionFind;
	QAction *actionReplace;
	QAction *actionGotoLine;
	QAction *actionFindNext;
	QAction *actionFindPrev;

	bool gotoLineEnabled;
	kateItemDataManager	*defColors;
	QeGtkSourceViewLangDef	*myLang;
	QeGTK_Highlighter	*highlight;

	QString			fileName;

	QTextEdit		*editor;
	QWidget			*lineNumber;
	QFrame			*editorContainer;
	QLayout			*editorLayout;

	QTextEdit		*editor2;
	QWidget			*lineNumber2;
	QFrame			*editorContainer2;
	QLayout			*editorLayout2;

	QSplitter		*splitter;

	QGridLayout		*layout;
	QWidget			*internalFind;
	QWidget			*internalReplace;
	QWidget			*internalGotoLine;
	
	Ui::InlineFind  	uiInlineFind;
	Ui::InlineReplace	uiInlineReplace;
	Ui::InlineGotoLine	uiInlineGotoLine;	
};

#endif // __TEXT_DISPLAY_H__
