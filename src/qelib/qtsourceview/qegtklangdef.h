#ifndef __QE_GTK_LANG_DEF_H__
#define __QE_GTK_LANG_DEF_H__

class QDomDocument;
class QString;
class QStringList;

class QDomNode;
class QDomNodeList;

///////////////////
struct  QeEntityDef
{
	QString type;
	QString name;
	QString style;
};

struct QeEntityString : QeEntityDef
{
	bool	atEOL;
	QString	startRegex;
	QString	endRegex;
};

struct QeEntityLineComment : QeEntityDef
{
	QString	start;
};

struct QeEntityBlockComment : QeEntityDef
{
	QString	startRegex;
	QString	endRegex;
};

struct QeEntityPatternItem : QeEntityDef
{
	QString regex;
};

struct QeEntityKeywordList : QeEntityDef
{
	bool		caseSensitive;
	bool		matchEmptyStringAtBeginning;
	bool		matchEmptyStringAtEnd;
	QString		startRegex;
	QString		endRegex;
	QStringList	list;
};


///////////////////

class QeGTK_Highlighter;

class QeGtkSourceViewLangDef
{
friend class QeGTK_Highlighter;

public:
	QeGtkSourceViewLangDef( QDomDocument doc );
	QeGtkSourceViewLangDef( QString fileName );
	virtual ~QeGtkSourceViewLangDef();

	bool	load( QString fileName );
	bool	load( QDomDocument doc );
	
private:
	bool	isTrue( QString s );

	bool	loadEntity(QDomNode node, QeEntityDef &entity );
	bool	loadLineComments( QDomNodeList nodes );
	bool	loadStrings( QDomNodeList nodes );
	bool	loadPatternItems( QDomNodeList nodes );
	bool	loadBlockComments( QDomNodeList nodes, QList<QeEntityBlockComment> &list );
	bool	loadKeywordList( QDomNodeList nodes );

	QString		name;
	QString		version;
	QString 	section;
	QStringList	mimeTypes;
	QStringList	extensions;

	QString				escapeChar;
	QList<QeEntityString>		stringsDefs; //
	QList<QeEntityLineComment>	lineCommentsDefs; //
	QList<QeEntityBlockComment>	blockCommentsDefs;
	QList<QeEntityKeywordList>	keywordListDefs; //
	QList<QeEntityPatternItem>	patternItems; //
	QList<QeEntityBlockComment>	syntaxItemDefs; //
};

#endif // __QE_GTK_LANG_DEF_H__

