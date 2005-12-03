#include <QList>
#include <QDir>
#include <QTextLayout>
#include "qecpphighlighter.h"

/**
 * \brief default constructor
 * @param d a pointer to the abstruct factory for item-datas
 * 
 * This will construct a CPP highlighter. It will use \p d as the
 * factory to get the highlight colro definitions for all the items 
 * found on the text.
 */
QECPPHighlighter::QECPPHighlighter( kateItemDataManager *d )
	:QECodeHighlighter()
{
	defaultColors = d;
	addMappings_cpp();
}

QColor QECPPHighlighter::getDefaultColor()
{
	if (defaultColors)
		return defaultColors->getItemData("dsNormal").getColor();
	else
		return Qt::black;
}

QColor QECPPHighlighter::getDefaultBackground()
{
	if (defaultColors)
		return defaultColors->getItemData("dsNormal").getBackground();
	else
		return Qt::white;
}

void QECPPHighlighter::highlightBlock(QTextBlock block)
{
	QTextLayout *layout = block.layout();
	const QString text = block.text();
	
	QList<QTextLayout::FormatRange> overrides;
	
	foreach (QEMyMapNode pattern, mappings.keys())
	{
		QRegExp expression(pattern.key);
		int i = text.indexOf(expression);
	
		while (i >= 0) 
		{
			QTextLayout::FormatRange range;
			range.start  = i;
			range.length = expression.matchedLength();
			range.format = pattern.value;
			overrides << range;
			i = text.indexOf(expression, i + expression.matchedLength());
		}
	}
	
	layout->setAdditionalFormats(overrides);
	const_cast<QTextDocument *>(block.document())->markContentsDirty( block.position(), block.length() );
}

void QECPPHighlighter::setHighlight( QString fileName )
{
	if (QDir::match( "*.cpp",fileName ) )
		addMappings_cpp();
	else if (QDir::match( "*.cpp",fileName ) )
		addMappings_cpp();
	else if (QDir::match( "*.c",fileName ) )
		addMappings_cpp();
	else if (QDir::match( "*.cxx",fileName ) )
		addMappings_cpp();
	else if (QDir::match( "*.h",fileName ) )
		addMappings_cpp();
	else if (QDir::match( "*.pro",fileName ) )
		addMappings_pro();
	else
		mappings.clear();
// 	else if (QDir::match( "*.cpp",fileName ) );

//	redraw the document
//	highlight( 0, 0, 0 );
}

void QECPPHighlighter::addMapping(const QString &pattern, const QTextCharFormat &format, bool fullWord )
{
	QString p = pattern;
	if (fullWord)
		p = "\\b" + p + "\\b";
		
	mappings.add( p, format );
}

void QECPPHighlighter::addMapping( const QString &pattern, const QString &formatName, bool fullWord )
{
	if (!defaultColors)
	{
		QTextCharFormat t;
		t.setBackground( Qt::white );
		t.setForeground( Qt::black );
		addMapping( pattern, t, fullWord );
	}
	else
		addMapping( pattern, defaultColors->getItemData(formatName).toCharFormat(), fullWord );
}

void QECPPHighlighter::addMappings_cpp()
{
	mappings.clear();
	// reserved words
	addMapping( "case"    , "dsKeyword", true );
	addMapping( "class"   , "dsKeyword", true );
	addMapping( "delete"  , "dsKeyword", true );
	addMapping( "else"    , "dsKeyword", true );
	addMapping( "for"     , "dsKeyword", true );
	addMapping( "if"      , "dsKeyword", true );
	addMapping( "private" , "dsKeyword", true );
	addMapping( "protected","dsKeyword", true );
	addMapping( "public"  , "dsKeyword", true );
	addMapping( "new"     , "dsKeyword", true );
	addMapping( "return"  , "dsKeyword", true );
	addMapping( "struct"  , "dsKeyword", true );
	addMapping( "switch"  , "dsKeyword", true );
	addMapping( "while"   , "dsKeyword", true );

	// some types
	addMapping( "int"     , "dsDataType", true );
	addMapping( "float"   , "dsDataType", true );
	addMapping( "double"  , "dsDataType", true );
	addMapping( "void"    , "dsDataType", true );
	addMapping( "char"    , "dsDataType", true );
	addMapping( "unsigned", "dsDataType", true );
	
// 	addMapping( "QString"     , "dsDataType", true );
// 	addMapping( "QWidget"     , "dsDataType", true );
// 	addMapping( "QPoint"      , "dsDataType", true );
// 	addMapping( "QTextBrowser", "dsDataType", true );
// 	addMapping( "QMainWindow" , "dsDataType", true );
// 	addMapping( "QLayout"     , "dsDataType", true );
// 	addMapping( "QMenu"       , "dsDataType", true );
// 	addMapping( "QAction"     , "dsDataType", true );
// 	addMapping( "QToolBar"    , "dsDataType", true );
// 	addMapping( "QToolButton" , "dsDataType", true );
//  	addMapping( "QObject"     , "dsDataType", true );
// 	addMapping( "", "dsDataType" );
	
	// numbers
	addMapping( "[0-9]+"         , "dsDecVal", true );
	addMapping( "0x[\\da-fA-F]+" , "dsDecVal", true );
	addMapping( "0[0-7]+"        , "dsBaseN" , true );
	addMapping( "[\\d]*\\.[\\d]*", "dsFloat" , true );

	// Qt4 extentions
	addMapping( "Q_OBJECT", "dsAlert", true );
	addMapping( "SLOT"    , "dsAlert", true );
	addMapping( "SIGNAL"  , "dsAlert", true );
	addMapping( "connect" , "dsAlert", true );
	addMapping( "slots"   , "dsAlert", true );
	addMapping( "foreach" , "dsAlert", true );
	
	// strings
	addMapping("'[^']+'"   , "dsChar" );
	addMapping("\"[^\"]*\"", "dsString" );

	// comments	
	addMapping( QString("%1[^\n]*").arg("#"), "dsOthers" );
	addMapping( QString("%1[^\n]*").arg("//"), "dsComment" );
	addMapping( QString("%1[^\n]*").arg("/\\*"), "dsComment" );
	addMapping( QString("%1[^\n]*").arg("///"), "dsError" );
	addMapping( QString("%1[^\n]*").arg("/\\*\\*"), "dsAlert" );
}

void QECPPHighlighter::addMappings_pro()
{
	mappings.clear();

	addMapping( "[A-Z_]+", "dsDataType", true );
	addMapping( "#[^\n]*", "dsComment" );
	addMapping( "\".*\"", "dsString", true );
	addMapping( "\\b[a-z0-9_]+\\(.*\\)", "dsFunction" );
}
