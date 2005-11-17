#ifndef __TEXT_DISPLAY_H__
#define __TEXT_DISPLAY_H__

#include <QTextDocument>
#include <QTextEdit>
#include <QAction>
#include <QVBoxLayout>

#include "ui_inlinefind.h"
#include "ui_inlinereplace.h"
#include "ui_inlinegotoline.h"

/**
 * \file    textdisplaycpp
 * \brief   Declaration of the textdisplay class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

class TextDisplay: public QWidget {
Q_OBJECT
public:
	TextDisplay( QTextEdit *editor, QWidget *parent=NULL );
	~TextDisplay();

	QTextEdit* getEditor() {return editor; };
	QWidget* getInternalFind() { return internalFind; };
	QTextDocument *document();
	
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
	QTextEdit *editor;
	QVBoxLayout *layout;
	QString fileName;
	QWidget *internalFind;
	QWidget *internalReplace;
	QWidget *internalGotoLine;
	
	Ui::InlineFind uiInlineFind;
	Ui::InlineReplace uiInlineReplace;
	Ui::InlineGotoLine uiInlineGotoLine;
};

#endif // __TEXT_DISPLAY_H__
