#pragma once

/**
 * \file qsvlangdef.h
 * \brief Definition of the language definition, and support structs
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvLangDef
 */

#include <QMap>
#include <QStringList>

class QDomDocument;
class QString;

class QDomNode;
class QDomNodeList;

///////////////////
struct  QsvEntityDef
{
	QString type;
	QString name;
	QString style;
};

struct QsvEntityString : QsvEntityDef
{
	bool	atEOL;
	QString	startRegex;
	QString	endRegex;
};

struct QsvEntityLineComment : QsvEntityDef
{
	QString	start;
};

struct QsvEntityBlockComment : QsvEntityDef
{
	QString	startRegex;
	QString	endRegex;
};

struct QsvEntityPatternItem : QsvEntityDef
{
	QString regex;
};

struct QsvEntityKeywordList : QsvEntityDef
{
	bool		caseSensitive;
	bool		matchEmptyStringAtBeginning;
	bool		matchEmptyStringAtEnd;
	QString		startRegex;
	QString		endRegex;
	QStringList	list;
};


///////////////////
class QsvLangDef
{
friend class QsvSyntaxHighlighter;
// friend class QeLangDefFactory;

public:
	QsvLangDef( QString fileName );
	QsvLangDef( QDomDocument doc );
	virtual ~QsvLangDef();

	bool	load( QString fileName );
	bool	load( QDomDocument doc );
	bool	isValid() const;

	QString getVersion() const;
	QString	getName() const;
	QString getSection() const;
	QStringList getMimeTypes() const;	
	
	static bool isTrue( QString s );
private:

	bool	loadEntity(QDomNode node, QsvEntityDef &entity );
	bool	loadLineComments( QDomNodeList nodes );
	bool	loadStrings( QDomNodeList nodes );
	bool	loadPatternItems( QDomNodeList nodes );
	bool	loadBlockComments( QDomNodeList nodes, QList<QsvEntityBlockComment> &list );
	bool	loadKeywordList( QDomNodeList nodes );

	QMap<QString,QString>		attributes;
	QStringList			mimeTypes;
	QStringList			extensions;

	QString				escapeChar;
	QList<QsvEntityString>		stringsDefs; 
	QList<QsvEntityLineComment>	lineCommentsDefs; 
	QList<QsvEntityBlockComment>	blockCommentsDefs;
	QList<QsvEntityKeywordList>	keywordListDefs; 
	QList<QsvEntityPatternItem>	patternItems; 
	QList<QsvEntityBlockComment>	syntaxItemDefs; 
};
