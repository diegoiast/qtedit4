#include "qelib/qtsourceview/qegtkhighlighter.h"
#include "qelib/qtsourceview/qegtklangdef.h"
#include "kateitemdatamanager.h"

QeGTK_Highlighter::QeGTK_Highlighter( QTextDocument *parent, kateItemDataManager *manager  )
	:QSyntaxHighlighter(parent)
{
	this->manager = manager;
}

QeGTK_Highlighter::QeGTK_Highlighter( QTextEdit *parent, kateItemDataManager *manager )
	:QSyntaxHighlighter(parent)
{
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
	foreach( QeEntityKeywordList l, lang->keywordListDefs )
	{
		foreach( QString s, l.list )
		{			

//			TODO use these defintions
// 			if (l.matchEmptyStringAtBeginning)
// 			if (l.matchEmptyStringAtEnd)

			s = l.startRegex + s;
			s = l.endRegex + s;
			addMapping( s, l.style, true );
		}
	}

	// syntax itmes...
	foreach( QeEntityBlockComment l, lang->syntaxItemDefs )
	{
// 		QString s = l.startRegex + QString("[^%1]*").arg(l.startRegex) + l.endRegex;
// 		QString s = l.startRegex + QString("[^%s]+").arg(l.endRegex);// + l.endRegex;
		
		// FIXME endRegex is generally "\n"... which is bad for us... 
		QString s = l.startRegex;//  + l.endRegex;
		addMapping( s, l.style );
	}

	// later, pattern items
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
	// block comments are handeled in the draing function	
	foreach( QeEntityLineComment l, lang->lineCommentsDefs )
	{
		addMapping( QString("%1[^\n]*").arg(l.start), l.style );
	}
	
}

// called when need to update a paragraph
void QeGTK_Highlighter::highlightBlock(const QString &text)
{
	// this code draws each line
	QOrderedMapNode<QString,QTextCharFormat> pattern;
	foreach ( pattern, mappings.keys())
	{
		QRegExp expression(pattern.key);
		int index = text.indexOf(expression);
		
		while (index >= 0) 
		{
			int length = expression.matchedLength();
			setFormat(index, length, pattern.value );
			index = text.indexOf(expression, index + length);
		}
	}

	setCurrentBlockState(0);

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

	addMapping( pattern, manager->getItemData(s).toCharFormat(), fullWord );
}
