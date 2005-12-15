#include <QString>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>

#include "qecodehighlighter.h"
#include "qecodeeditor.h"


/**
 * \class QECodeEditor
 * \brief A code editor, with syntax highlighter
 *
 * A more sane editor-widget for editing texts. Much more advanced then
 * QTextEdit. The highligting is done via another class, whis is passed in then
 * constructor.
 * 
 * This is the lower level class to edit the text itself. The highlievel
 * widget which you usually want to add to your application is the TextDisplay
 * which is smart enough to know about saving files, searches etc.
 * 
 * \see TextDisplay
 * \see QTextEdit
 */


/**
 * \brief default constructor
 * \param p is passed to QTextEditor::QTexteditor
 * \param h is the syntax highlighter used in this editor
 * 
 * constructor, builds a QECodeEditor from QTextEdit, and sets some sane defaults.
 *  - sets wordWrap to QTextEdit::NoWrap
 *  - sets textFormat to Qt::PlainText
 *  - sets fileName to "";
 *  - sets codeHighlighter to show C++
 */
QECodeEditor::QECodeEditor( QWidget *p, QECodeHighlighter *h ):QTextEdit(p)
{
	setLineWrapMode( QTextEdit::NoWrap );

	fileName        = "";
	codeHighlighter = h;

	if (codeHighlighter)
		codeHighlighter->addToDocument( document() );

	setHighlight( "1.cpp" );
}



/**
 * \brief destructs a QECodeEditor
 * 
 * destructor, destructs a QECodeEditor. Since it's a virutal 
 * desctructor, after this call QTextEdit::~QTextEdit desctructor will
 * be callled.
 */
QECodeEditor::~QECodeEditor()
{
	delete codeHighlighter;
	codeHighlighter = NULL;
}


/**
 * \brief return the name of file currently beeing edited.
 * 
 * This is a way for you got the the property fileName. 
 * 
 * \see saveFile( fileName )
 */
QString QECodeEditor::getFileName()
{
	return fileName;
}

/**
 * \brief load a file, and set it's text into the editor.
 * \param fileName the file to be loaded
 *
 * Load a file from the drive into this editor.
 *  - set correctly the file name,
 *  - load the text from the physical file into the buffer
 *  - find a suitable highlighter (will redraw the screen)
 *
 * \see QECodeEditor::loadFile( QString fileName, QTextCodec c )
 */
bool QECodeEditor::loadFile( QString fileName )
{
	return loadFile( fileName, QTextCodec::codecForLocale() );
}

/**
 * \brief load a file, and set it's text into the editor.
 * \param fileName the file to be loaded
 * \param c a charset to assume when loading the file
 *
 * Bevaves like the above function, excepct you can choose an encoding 
 * in this the file is saved.
 */
bool QECodeEditor::loadFile( QString fileName, QTextCodec *c )
{
	QFile f( fileName );
	
	if (!f.open( QIODevice::ReadOnly ) )
	{
		return false;
	}
	
	QTextStream t(&f);
	t.setCodec( c );
	setPlainText( t.readAll() );
	f.close();
	
	setHighlight( fileName );
	this->fileName = fileName;
	return true;
}


bool QECodeEditor::saveFile( QString fileName )
{
	return saveFile( fileName, QTextCodec::codecForLocale() );
}

/*
 * Save the file on the disk with the same fileName.
 * If fileName is empty, it will call SaveAs().
 *
 */
bool QECodeEditor::saveFile( QString fileName, QTextCodec *c )
{
	QFile f( fileName );

	if (!f.open( QIODevice::WriteOnly ) )
	{
		return false;
	}
	
	QTextStream t(&f);
	t.setCodec( c );
	t << toPlainText();
	f.close();

	this->fileName = fileName;
	setHighlight( fileName );
	return true;
}


/* 
 * change the highlight type of this editor and refresh the display.
 */
void QECodeEditor::setHighlight( QString fileName )
{
	// set default color and background
	setTextColor( codeHighlighter->getDefaultColor() );

	// set default background
	QPalette p( palette() );
	p.setColor( QPalette::Base, codeHighlighter->getDefaultBackground() );
	setPalette( p );
	
	//	qDebug("QECodeEditor::setHighlight(%x)", h );
	if (codeHighlighter)
		codeHighlighter->setHighlight( fileName );

	update();
}
