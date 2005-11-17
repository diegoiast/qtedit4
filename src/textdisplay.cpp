#include <QtDebug>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>

#include "textdisplay.h"

/**
 * \file    textdisplaycpp
 * \brief   Implementation of the textdisplay class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

TextDisplay::TextDisplay( QTextEdit *editor, QWidget *parent ):
	QWidget(parent)
{
	this->editor = editor;
	
	internalFind= new QWidget();
	uiInlineFind.setupUi( internalFind );
	internalFind->hide();
	
	internalReplace= new QWidget();
	uiInlineReplace.setupUi( internalReplace );
	internalReplace->hide();

	internalGotoLine = new QWidget;
	uiInlineGotoLine.setupUi( internalGotoLine );
	internalGotoLine->hide();
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin( 1 );
	layout->setSpacing( 1 );
	layout->addWidget( this->editor );
	layout->addWidget( this->internalFind );
	layout->addWidget( this->internalReplace );
	layout->addWidget( this->internalGotoLine );
	this->setLayout( layout );

	// connect the find slots
 	connect( uiInlineFind.editSearchText, SIGNAL(textChanged(QString)), this, SLOT(incrementalSearch(QString)) );
	connect( uiInlineFind.buttonHide, SIGNAL(clicked()), internalFind, SLOT(hide()) );
	connect( uiInlineFind.buttonNext, SIGNAL(clicked()), this, SLOT(findNext()) ) ;
	connect( uiInlineFind.buttonPrevious, SIGNAL(clicked()), this, SLOT(findPrev()) );
//	FIXME should enter search again? or hide the widget?
// 	connect( uiInlineFind.editSearchText, SIGNAL(returnPressed ()),this, SLOT(findNext()) ) ;
 	connect( uiInlineFind.editSearchText, SIGNAL(returnPressed()),this, SLOT(find()) ) ;

	// connect the replace slots
	connect( uiInlineReplace.buttonHide, SIGNAL(clicked()), this, SLOT(replace()) ) ;
	connect( uiInlineReplace.editSearchText, SIGNAL(textChanged(QString)), this, SLOT(incrementalSearch(QString)) );
	connect( uiInlineReplace.editSearchText, SIGNAL(returnPressed()),this, SLOT(doReplace()) ) ;
	connect( uiInlineReplace.editReplaceText, SIGNAL(returnPressed()),this, SLOT(doReplace()) ) ;

	// connect the goto line slots
	connect( uiInlineGotoLine.buttonHide, SIGNAL(clicked()), this, SLOT(gotoLine()) );
	connect( uiInlineGotoLine.sbLineNumber, SIGNAL(editingFinished()), internalGotoLine, SLOT(hide()) );
	connect( uiInlineGotoLine.sbLineNumber, SIGNAL(valueChanged ( int )), this, SLOT(gotoLine(int )) );
}

TextDisplay::~TextDisplay()
{
	delete editor;
	delete internalFind;
	delete internalReplace;
}

QTextDocument *TextDisplay::document()
{
	return editor->document();
}

void TextDisplay::hideInternalWidgets()
{
	internalFind->hide();
	internalReplace->hide();
	internalGotoLine->hide();
}

void TextDisplay::undo()
{
	editor->document()->undo();
}

void TextDisplay::redo()
{
	editor->document()->redo();
}

void TextDisplay::copy()
{
	editor->copy();
}

void TextDisplay::cut()
{
	editor->paste();
}

void TextDisplay::paste()
{
	editor->paste();
}

void TextDisplay::find()
{
	if (internalFind->isVisible())
	{
		internalFind->hide();
		editor->setFocus();
	}
	else
	{
		hideInternalWidgets();
		internalFind->show();
		uiInlineFind.editSearchText->selectAll();
		uiInlineFind.editSearchText->setFocus();
	}
}

void TextDisplay::replace()
{
	if (internalReplace->isVisible())
	{
		internalReplace->hide();
		editor->setFocus();
	}
	else
	{
		hideInternalWidgets();
		internalReplace->show();
		uiInlineReplace.editSearchText->setFocus();
	}
}

void TextDisplay::gotoLine()
{
	if (internalGotoLine->isVisible())
	{
		internalGotoLine->hide();
		editor->setFocus();
	}
	else
	{
		hideInternalWidgets();
		internalGotoLine->show();
		uiInlineGotoLine.sbLineNumber->setFocus();

		// setup the goto line thingie
		uiInlineGotoLine.sbLineNumber->setMinimum( 1 );
		uiInlineGotoLine.sbLineNumber->setMaximum( getLineCount() );
		uiInlineGotoLine.sbLineNumber->setValue( getCursorLocation().y() );
// 		FIXME: bug in qt?
// 		uiInlineGotoLine.sbLineNumber->lineEdit()->selectAll();
	}
}

void TextDisplay::doReplace()
{
// 	if (wholeDocument)
// 	{
// 		QTextCursor c = editor->textCursor();
// 		c.movePosition(QTextCursor::Start);
// 		editor->setTextCursor(c);
// 	}

	QTextDocument::FindFlags findOptions = 0;
	QString s = uiInlineReplace.editSearchText->text();
	QPalette p( uiInlineFind.editSearchText->palette() );

	if (uiInlineReplace.cbReplaceAll->isChecked())
	{
		// do multiple searches
		int n = 0;
		while (editor->find(s,findOptions))
		{
			editor->insertPlainText( uiInlineReplace.editReplaceText->text() );
			n++;
		}

		if (n!=0)
			p.setColor( QPalette::Base, QColor(Qt::yellow).light() );
		else
			p.setColor( QPalette::Base, QColor(Qt::red).light() );

		uiInlineReplace.editSearchText->setPalette( p );
	}
	else
	{
		// replace only one
		if (editor->find(s,findOptions))
		{
			editor->insertPlainText( uiInlineReplace.editReplaceText->text() );
			p.setColor( QPalette::Base, QColor(Qt::yellow).light() );
		}
		else
		{
			p.setColor( QPalette::Base, QColor(Qt::red).light() );
		}
		uiInlineReplace.editSearchText->setPalette( p );
	}
}

void TextDisplay::findNext()
{
	if ( uiInlineFind.editSearchText->text().isEmpty() )
	{
		find();
		return;
	}
	
	if (uiInlineFind.cbMatchCase->isChecked())
		findString( uiInlineFind.editSearchText->text(), false, QTextDocument::FindCaseSensitively );
	else
		findString( uiInlineFind.editSearchText->text(), false );
}

void TextDisplay::findPrev()
{
	if ( uiInlineFind.editSearchText->text().isEmpty() )
	{
		find();
		return;
	}
	
	QTextDocument::FindFlags flags = QTextDocument::FindBackward;
	if (uiInlineFind.cbMatchCase->isChecked())
		flags |= QTextDocument::FindCaseSensitively;
	
	findString( uiInlineFind.editSearchText->text(), false, flags );
}

void TextDisplay::incrementalSearch( QString s )
{
	if (uiInlineFind.cbMatchCase->isChecked())
		findString( s, true, QTextDocument::FindCaseSensitively );
	else
		findString( s, true );
}

QPoint TextDisplay::getCursorLocation()
{
	QTextDocument *doc = editor->document();
	QTextBlock block = doc->begin();
	int cursorOffset = editor->textCursor().position();
	int off = 0;
	int x=1, y=1;

	while( cursorOffset>=(off + block.length()) )
	{
		off += block.length();
		block = block.next();
		y+= 1;
	}
	x = cursorOffset - off +1;

 	return QPoint( x, y );
}

void TextDisplay::setCursorLocation( QPoint p )
{
	QTextDocument *doc = editor->document();
	QTextBlock block = doc->begin();
	int off = 0;
	int y=1;

	for( ; y<p.y(); y++ )
	{
		off += block.length();
		
		if (block != doc->end())
			block = block.next();
		else
			return;
	}
	
	off+= p.x();

	QTextCursor c = editor->textCursor();
	c.setPosition( off );
	editor->setTextCursor( c );
}

void TextDisplay::setCursorLocation( int x, int y )
{
	setCursorLocation( QPoint(x,y) );
}

uint TextDisplay::getLineCount()
{
	QTextDocument *doc = editor->document();
	QTextBlock block = doc->begin();
	int y=1;

	for( block=doc->begin(); block!= doc->end(); block = block.next())
		y++;

 	return y;
}


void TextDisplay::findString( QString s, bool wholeDocument, QTextDocument::FindFlags findOptions )
{
	if ( s.isEmpty() )
	{
		editor->find( "" );
		return;
	}
	
	QPalette p( uiInlineFind.editSearchText->palette() );

	if (wholeDocument)
	{
		QTextCursor c = editor->textCursor();
		c.movePosition(QTextCursor::Start);
		editor->setTextCursor(c);
	}
	
	if (editor->find(s,findOptions) )
		p.setColor( QPalette::Base, QColor(Qt::yellow).light() );
	else
		p.setColor( QPalette::Base, QColor(Qt::red).light() );

	// modify the background color of the active widget, only
	if (internalFind->isVisible())
		uiInlineFind.editSearchText->setPalette( p );
	else
		uiInlineReplace.editSearchText->setPalette( p );
		
	//setBackgroundRole( QPalette::AlternateBase );
}

void TextDisplay::gotoLine( int i )
{
	setCursorLocation( 0, i );
}
