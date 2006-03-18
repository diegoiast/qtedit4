#include "qegtkhighlighter.h"
#include "qegtklangdef.h"
#include "kateitemdatamanager.h"

#include "debug_info.h"

QeGTK_Highlighter::QeGTK_Highlighter( QTextDocument *parent, kateItemDataManager *manager  )
	:QSyntaxHighlighter(parent)
{
	language = NULL;
	this->manager = manager;
}

QeGTK_Highlighter::QeGTK_Highlighter( QTextEdit *parent, kateItemDataManager *manager )
	:QSyntaxHighlighter(parent)
{
	language = NULL;
	this->manager = manager;
}


void QeGTK_Highlighter::setHighlight( QeGtkSourceViewLangDef *lang )
{
	QString str;

	language = lang;

	mappings.clear();
	
	if (lang == NULL)
		return;

	//first match keyword lists
	// TODO: optimizations
	foreach( QeEntityKeywordList l, lang->keywordListDefs )
	{
		foreach( QString s, l.list )
		{
//			TODO use these defintions
// 			if (l.matchEmptyStringAtBeginning)
// 			if (l.matchEmptyStringAtEnd)

			s = l.startRegex + s + l.endRegex;
// 			qDebug( qPrintable(s) );
			addMapping( s, l.style );
		}
	}

	// syntax itmes...
	foreach( QeEntityBlockComment l, lang->syntaxItemDefs )
	{
		QString s;
		if (l.endRegex == "\\n")
			s  = l.startRegex + ".*$";
		else
			s  = l.startRegex + ".*" + l.endRegex;
 		addMapping( s, l.style );
	}

	// later, pattern items
	// TODO: optimizations
	foreach( QeEntityPatternItem l, lang->patternItems )
	{
		addMapping( l.regex, l.style, !true );
	}

	// strings...
	foreach( QeEntityString l, lang->stringsDefs )
	{
		if (!l.atEOL)
			continue;

		QString s = l.startRegex + QString("[^%1]*").arg(l.startRegex) + l.endRegex;
		addMapping( s, l.style );
	}

	// and finally... line comments...
	// block comments are handeled in the drawing function	
	foreach( QeEntityLineComment l, lang->lineCommentsDefs )
	{
		addMapping( QString("%1.*").arg(l.start), l.style );
	}
}

// called when need to update a paragraph
void QeGTK_Highlighter::highlightBlock(const QString &text)
{
	if (language == NULL)
		return;

	QOrderedMapNode<QString,QTextCharFormat> pattern;
	
	// optimizations...
	if (text.simplified().isEmpty())
		goto HANDLE_BLOCK_COMMENTS;

	foreach( QeEntityLineComment l, language->lineCommentsDefs )
	{
		if (text.startsWith( l.start ))
		{
			setFormat( 0, text.length(), manager->getItemData("dsComment").toCharFormat() );
			return;
		}
	}

	// this code draws each line
	foreach ( pattern, mappings.keys())
		drawText( text, pattern.key, pattern.value );

	setCurrentBlockState(0);

HANDLE_BLOCK_COMMENTS:
	// what if not block comments defined...?
	if (language->blockCommentsDefs.count() == 0)
		return;

	QRegExp startExpression( language->blockCommentsDefs.at(0).startRegex );
	QRegExp endExpression  ( language->blockCommentsDefs.at(0).endRegex );
	
	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(startExpression);
	
	while (startIndex >= 0) 
	{
		int endIndex = text.indexOf(endExpression, startIndex);
		int commentLength;
		if (endIndex == -1) 
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} 
		else 
		{
			commentLength = endIndex - startIndex
				+ endExpression.matchedLength();
		}
		setFormat( startIndex, commentLength, manager->getItemData("dsComment").toCharFormat() );
		startIndex = text.indexOf(startExpression, startIndex + commentLength);
	}
}

void QeGTK_Highlighter::addMapping(const QString &pattern, const QTextCharFormat &format, bool fullWord )
{	
	QString p = pattern;
	if (fullWord)
		p = "\\b" + p + "\\b";
		
#ifdef __DEBUG_ADD_MAPPING__
	qDebug( "QeGTK_Highlighter::addMapping - [%s]", qPrintable(pattern) );
#endif

	mappings.add( p, format );
}

void QeGTK_Highlighter::addMapping(const QString &pattern, const QString formatName, bool fullWord )
{
	QString s = formatName;
	if (!manager)
		return;

	// convert GTK formats to Kate
	if (s == "Comment")
		s = "dsComment";
	else if (s == "String")
		s = "dsString";
	else if (s == "Preprocessor")
		s = "dsOthers";
	else if (s == "Keyword")
		s = "dsKeyword";
	else if (s == "Data Type")
		s = "dsDataType";
	else if (s == "Decimal")
		s = "dsDecVal";
	else if (s == "Floating Point")
		s = "dsFloat";
	else if (s == "Base-N Integer")
		s = "dsBaseN";
	else if (s == "Function")
		s = "dsFunction";
	else if (s == "Others 2")
		s = "dsOthers2";
	else if (s == "Others 3")
		s = "dsOthers3";

	addMapping( pattern, manager->getItemData(s).toCharFormat(), fullWord );
}

void QeGTK_Highlighter::drawText( QString text, QString s, QTextCharFormat &format )
{
	if (s.contains( QRegExp("[^*+()?]") ))
		drawRegExp( text, s, format );
	else
		drawKeywords( text, s, format );
}

void QeGTK_Highlighter::drawRegExp( QString text, QString s, QTextCharFormat &format )
{	
	QRegExp expression(s);
	int index = text.indexOf(expression);

#ifdef __DEBUG_HIGHLIGHT__
	qDebug( "QeGTK_Highlighter::drawRegExp( [%s] )", qPrintable(s) );
#endif

	while (index >= 0)
	{
		int length = expression.matchedLength();
		setFormat(index, length, format );
		index = text.indexOf(expression, index + length);
	}
}

void QeGTK_Highlighter::drawKeywords( QString text, QString s, QTextCharFormat &format )
{
#ifdef __DEBUG_HIGHLIGHT__
	qDebug( "QeGTK_Highlighter::drawKeywords( [%s] )", qPrintable(s) );
#endif

	int index = text.indexOf(s);
	int length = s.length();
	int txtLen = text.length();
	
	while (index >= 0)
	{
		// paint keyword, only if its sorrounded by white chars
		// regexp are bad :)
#if 1		
		if (
		   ((index==0)             || (!text[index-1].isLetterOrNumber())) &&
		   ((index+length>=txtLen) || (!text[index+length].isLetterOrNumber()))
		   )
#else
		if (
		   ((index==0)             || (!text[index-1].isLetterOrNumber())) &&
		   ((index+length>=txtLen) || (!text[index+length].isLetterOrNumber()))
		   )
#endif		   
			setFormat(index, length, format );
		index = text.indexOf(s, index + length);
	}
}

