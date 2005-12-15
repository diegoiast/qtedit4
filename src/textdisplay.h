#ifndef __TEXT_DISPLAY_H__
#define __TEXT_DISPLAY_H__

#include <QTextDocument>
#include <QTextEdit>
#include <QAction>
#include <QVBoxLayout>

#include "qexdilib/qexitabwidget.h"


#include "ui_inlinefind.h"
#include "ui_inlinereplace.h"
#include "ui_inlinegotoline.h"

/**
 * \file    textdisplaycpp
 * \brief   Declaration of the textdisplay class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

class TextDisplay: public QWidget, public QITabInterface
{
Q_OBJECT
public:
	TextDisplay( QTextEdit *editor, QWidget *parent=NULL );
	~TextDisplay();

	QTextEdit* getEditor() {return editor; };
	QWidget* getInternalFind() { return internalFind; };
	QTextDocument *document();

protected:
	void createActions();
	
public slots:
	void hideInternalWidgets();
	void undo();
	void redo();
	void copy();
	void cut();
	void paste();
	void find();
	void replace();
	void gotoLine();
	void findNext();
	void findPrev();
	void incrementalSearch( QString s );
	void findString( QString s, bool wholeDocument=false, QTextDocument::FindFlags findOptions=0 );
	void doReplace();
	void gotoLine( int i );

	QPoint getCursorLocation();
	void setCursorLocation( QPoint p );
	void setCursorLocation( int x, int y );
 	uint getLineCount();
	
private:
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

	QString			fileName;
	QTextEdit		*editor;
	QVBoxLayout		*layout;
	QWidget			*internalFind;
	QWidget			*internalReplace;
	QWidget			*internalGotoLine;
	
	Ui::InlineFind  	uiInlineFind;
	Ui::InlineReplace	uiInlineReplace;
	Ui::InlineGotoLine	uiInlineGotoLine;
};

#endif // __TEXT_DISPLAY_H__
