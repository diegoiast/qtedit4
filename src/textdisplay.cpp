#include <QtDebug>

#include <QSplitter>
#include <QFile>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QClipboard>
#include <QTextEdit>
#include <QTimer>
#include <QThread>

#include "kateitemdatamanager.h"
#include "editorsettings.h"
#include "texteditorex.h"
#include "linenumberwidget.h"
#include "textdisplay.h"
#include "qtsourceview/qegtklangdef.h"
#include "qtsourceview/qelangdeffactory.h"
#include "qtsourceview/qegtkhighlighter.h"

/**
 * \file    textdisplaycpp
 * \brief   Implementation of the textdisplay class
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

TextDisplay::TextDisplay( QTextEdit *editor, QWidget *parent, kateItemDataManager *colors, bool ignoreConf ):
	QWidget(parent)
{
	defColors = colors;

	internalFind = new QWidget();
	uiInlineFind.setupUi( internalFind );
	internalFind->hide();
	
	internalReplace = new QWidget();
	uiInlineReplace.setupUi( internalReplace );
	internalReplace->hide();

	internalGotoLine = new QWidget;
	uiInlineGotoLine.setupUi( internalGotoLine );
	internalGotoLine->hide();

	highlight	= NULL;
	layout		= NULL;
	splitter	= NULL;

	this->editor	= NULL;
	lineNumber	= NULL;
	editorContainer	= NULL;
	editorLayout	= NULL;

	editor2		= NULL;
	lineNumber2	= NULL;
	editorContainer2= NULL;
	editorLayout2	= NULL;

	createActions();
	setEditor( editor );
	this->editor->setFocus();

	if (!ignoreConf)
	{
		EditorSettings *editSettings = EditorSettings::getInstance();
		connect( editSettings, SIGNAL(updateSettings()), this, SLOT(updateConfiguration()));
	}
}

TextDisplay::~TextDisplay()
{
	if (editor)
		delete editor;

	delete internalFind;
	delete internalReplace;
}

void TextDisplay::createActions()
{
	// edit menu
	actionUndo = new QAction( QIcon(":images/undo.png"), tr("&Undo"), this);
	actionUndo->setShortcut(tr("Ctrl+Z"));
	actionUndo->setStatusTip(tr("Undo the last operation"));
	connect( actionUndo, SIGNAL(triggered()), this, SLOT(undo()) );
	
	actionRedo = new QAction( QIcon(":images/redo.png"), tr("&Redo"), this);
	actionRedo->setShortcut(tr("Ctrl+Y"));
	actionRedo->setStatusTip(tr("Redo the last operation"));
	connect( actionRedo, SIGNAL(triggered()), this, SLOT(undo()) );
	
	actionCut = new QAction( QIcon(":images/editcut.png"), tr("Cu&t"), this);
	actionCut->setShortcut(tr("Ctrl+X"));
	actionCut->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
	connect( actionCut, SIGNAL(triggered()), this, SLOT(cut()) );
	
	actionCopy = new QAction( QIcon(":images/editcopy.png"), tr("&Copy"), this);
	actionCopy->setShortcut(tr("Ctrl+C"));
	actionCopy->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
	connect( actionCopy, SIGNAL(triggered()), this, SLOT(copy()) );
	
	actionPaste = new QAction( QIcon(":images/editpaste.png"), tr("&Paste"), this);
	actionPaste->setShortcut(tr("Ctrl+V"));
	actionPaste->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
	actionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
	connect( actionPaste, SIGNAL(triggered()), this, SLOT(paste()) );

	// search menu
	actionFind = new QAction( QIcon(":images/find.png"), tr("&Find"), this);
	actionFind->setShortcut(tr("Ctrl+F"));
	actionFind->setStatusTip(tr("Search for a string in the currently opened window"));
	connect( actionFind, SIGNAL(triggered()), this, SLOT(find()) );
	
	actionReplace = new QAction( QIcon(":images/find.png"), tr("&Replace"), this);
	actionReplace->setShortcut(tr("Ctrl+R"));
	actionReplace->setStatusTip(tr("Replace a string in the currently opened window"));
	connect( actionReplace, SIGNAL(triggered()), this, SLOT(replace()) );
	
	actionGotoLine = new QAction( QIcon(":images/find.png"), tr("&Goto line"), this);
	actionGotoLine ->setShortcut(tr("Ctrl+G"));
	actionGotoLine ->setStatusTip(tr("Scroll to a new line in the currently opened window"));
	connect( actionGotoLine, SIGNAL(triggered()), this, SLOT(gotoLine()) );
	
	actionFindNext = new QAction( QIcon(":images/next.png"), tr("&Find next"), this);
	actionFindNext->setShortcut(tr("F3"));
	actionFindNext->setStatusTip(tr("Search for the next match of a string the currently opened window"));
	connect( actionFindNext, SIGNAL(triggered()), this, SLOT(findNext()) );

	actionFindPrev = new QAction( QIcon(":images/previous.png"), tr("&Find previous"), this);
	actionFindPrev->setShortcut(tr("Shift+F3"));
	actionFindPrev->setStatusTip(tr("Search for the previous match of a string the currently opened window"));
	connect( actionFindPrev, SIGNAL(triggered()), this, SLOT(findPrev()) );

// 	connect the find slots
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

void    TextDisplay::setEditor( QTextEdit *e, bool ignoreConf, bool isGotoLineEnabled )
{
	// clean ups
	delete editorLayout;
	delete editorLayout2;
	delete layout;
	delete splitter;

	layout		= NULL;
	splitter	= NULL;
	editor		= NULL;
	lineNumber	= NULL;
	editorContainer	= NULL;
	editorLayout	= NULL;
	editor2		= NULL;
	lineNumber2	= NULL;
	editorContainer2= NULL;
	editorLayout2	= NULL;


	layout		= new QGridLayout;
	splitter	= new QSplitter;
	layout->setMargin( 0 );
	layout->setSpacing( 0 );

	// if no editor has been passed, generate a twin set of editors
	// which share the same document. connect them using a splitter.
	// 
	// otherwise, just insert that editor (aside a line numbers widget)
	if (e == NULL)
	{
		// bottom editor - this one leads
		editorContainer = new QFrame;
		editorLayout	= new QHBoxLayout;
		editor		= new TextEditorEx;
		lineNumber	= new LineNumberWidget( editor );

		editorContainer->setFrameShape( QFrame::StyledPanel );
		editor->setLineWrapMode( QTextEdit::NoWrap );
		editorLayout->setMargin( 0 );
		editorLayout->setSpacing( 0 );
		editorLayout->addWidget( lineNumber );
		editorLayout->addWidget( editor );
		editorContainer->setLayout( editorLayout );
	
		// upper editor - hidden by default
		editorContainer2 = new QFrame;
		editorLayout2	= new QHBoxLayout;
		editor2		= new TextEditorEx;
		lineNumber2	= new LineNumberWidget( editor2 );

		editorContainer2->setFrameShape( QFrame::StyledPanel );
		editor2->setLineWrapMode( QTextEdit::NoWrap );
		editorLayout2->setMargin( 0 );
		editorLayout2->setSpacing( 0 );
		editorLayout2->addWidget( lineNumber2 );
		editorLayout2->addWidget( editor2 );
		editorContainer2->setLayout( editorLayout2 );
		editor2->setDocument( editor->document() );

		splitter->setOrientation( Qt::Vertical );
		splitter->setFrameShape( QFrame::NoFrame );
		splitter->addWidget( editorContainer2 );
		splitter->addWidget( editorContainer );

		// this code just hides the upper editor, and
		// shows the bottom one
		QList<int> n;
		n << 0;
		n << 1;
		splitter->setSizes( n );
		layout->addWidget( splitter );

		if (defColors!=NULL)
		{
			QPalette p( editor->palette() );
			p.setColor( QPalette::Base, defColors->getItemData("dsNormal").getBackground() );
			editor->setTextColor( defColors->getItemData("dsNormal").getColor() );
			editor2->setTextColor( defColors->getItemData("dsNormal").getColor() );
			editor->setPalette( p );
			editor2->setPalette( p );
		}
	}
	else
	{
		editor = e;
		actionGotoLine->setEnabled( isGotoLineEnabled );
		actionGotoLine->setVisible( isGotoLineEnabled );
		if (isGotoLineEnabled)
		{
			lineNumber = new LineNumberWidget( editor );
			layout->addWidget( lineNumber, 1, 0 );
			layout->addWidget( editor    , 1, 1 );
		}
		else
			layout->addWidget( editor );

		if (defColors!=NULL)
		{
			QPalette p( editor->palette() );
			p.setColor( QPalette::Base, defColors->getItemData("dsNormal").getBackground() );
			editor->setTextColor( defColors->getItemData("dsNormal").getColor() );
			editor->setPalette( p );
		}
	}
	
	actionCopy->setEnabled( false );
	actionCut->setEnabled( false );

	layout->addWidget( this->internalFind );
	layout->addWidget( this->internalReplace );
	layout->addWidget( this->internalGotoLine );

	connect( editor, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)) );
	connect( editor, SIGNAL(copyAvailable(bool)), actionCut , SLOT(setEnabled(bool)) );
	connect( actionRedo, SIGNAL(triggered()), this, SLOT(undo()) );

	this->setLayout( layout );
	editor->setFocus();

	if (!ignoreConf)
		updateConfiguration();

	// menus used by this widget
	if (editor->isReadOnly())
	{
		toolbars["Text operations"]->addAction( actionPaste );
		toolbars["Text operations"]->addAction( actionFind );
		toolbars["Text operations"]->addAction( actionGotoLine );
	}
	else
	{
		menus["&Edit"]->addAction( actionCopy );
		menus["&Search"]->addAction( actionFind );
		menus["&Search"]->addAction( actionGotoLine );
		menus["&Search"]->addSeparator();
		menus["&Search"]->addAction( actionFindNext );
		menus["&Search"]->addAction( actionFindPrev );

		toolbars["Text operations"]->addAction( actionCopy );
		toolbars["Text operations"]->addAction( actionCut );
		toolbars["Text operations"]->addAction( actionPaste );
		toolbars["Text operations"]->addAction( actionFind );
		toolbars["Text operations"]->addAction( actionReplace );
		toolbars["Text operations"]->addAction( actionGotoLine );
	}

}


QTextDocument *TextDisplay::document()
{
	return editor->document();
}

void	TextDisplay::setColorDef( kateItemDataManager *newColors )
{
	defColors = newColors;
	delete( highlight );
	highlight = new QeGTK_Highlighter( editor, defColors );
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

bool	TextDisplay::loadFile( QString fileName )
{
	this->fileName = fileName;	
	myLang = NULL;
	delete ( highlight );
	highlight = NULL;

	editor->clear();

	QTimer::singleShot( 0, this, SLOT(loadingTimer()));

	return true;
}

void TextDisplay::setGotoLineEnabled( bool enabled )
{
	gotoLineEnabled = enabled;
	actionGotoLine->setEnabled( enabled );
	actionGotoLine->setVisible( enabled );

	lineNumber->setVisible( enabled );
	lineNumber2->setVisible( enabled );
}

void TextDisplay::loadingTimer()
{
#if 1
	// this code loads the file in 2 timers:
	//  - the first will load the text, and then will issue a new timer
	//  - on second timer, the syntax highlighter will be applied
	if (highlight == NULL)
	{
		QFile f( fileName );
		if (!f.open( QIODevice::ReadOnly ) )
		{
			QTimer::singleShot( 0, this, SLOT(loadingTimer()));
			return;
		}
	
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QTextStream t(&f);
		editor->setPlainText( t.readAll() );
		f.close();
		highlight = new QeGTK_Highlighter( editor, defColors );
		QTimer::singleShot( 0, this, SLOT(loadingTimer()));
	}
	else
	{
		myLang = QeLangDefFactory::getInstanse()->getHighlight( fileName );
		highlight->setHighlight( myLang );
		highlight->setDocument( editor->document() );

		QApplication::restoreOverrideCursor();
	}
#endif

#if 0
	// this code loads the file in a single timer
	QFile f( fileName );
	
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (f.open( QIODevice::ReadOnly|QIODevice::Text) )
	{
		QTextStream t(&f);
		editor->setPlainText( t.readAll() );
		f.close();
	}
	myLang = QeLangDefFactory::getInstanse()->getHighlight( fileName );
	highlight = new QeGTK_Highlighter( editor, defColors );
	highlight->setHighlight( myLang );
	QApplication::restoreOverrideCursor();

#endif

#if 0
//	the difference beteewn those 2 methods, is that after a very small timers
//	you will see the text on screen, even tough it's not ready yet.
//	it feels faster :)
#endif

}

void	TextDisplay::updateConfiguration()
{
	EditorSettings *editSettings = EditorSettings::getInstance();

	// line numbers
	lineNumber->setVisible( editSettings->showLineNumbers );
	if (lineNumber2)
		lineNumber2->setVisible( editSettings->showLineNumbers );

	TextEditorEx *e = qobject_cast<TextEditorEx*>(editor);
	if (e)
	{
		// highlight current line
		e->setHightlighCurrentLine( editSettings->markCurrentLine );
		// wrapping	
		e->setLineWrapMode( editSettings->lineWrap? QTextEdit::WidgetWidth : QTextEdit::NoWrap );
	}

	e = qobject_cast<TextEditorEx*>(editor2);
	if (e)
	{
		// highlight current line
		e->setHightlighCurrentLine( editSettings->markCurrentLine );
		// wrapping	
		e->setLineWrapMode( editSettings->lineWrap? QTextEdit::WidgetWidth : QTextEdit::NoWrap );
	}

	// set font for editors	
	editor->setFont( editSettings->font );
	if (editor2)
		editor2->setFont( editSettings->font );
}
