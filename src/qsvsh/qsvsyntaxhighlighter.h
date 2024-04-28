#pragma once

/**
 * \file qsvsyntaxhighlighter.h
 * \brief Definition of the syntax highlighter
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see QsvSyntaxHighlighter
 */

#include <QSyntaxHighlighter>
#include "qorderedmap.h"

class QTextEdit;
class QTextCharFormat;
class QPlainTextEdit;
class QsvLangDef;
class QsvColorDefFactory;

struct QsvLanguageEntity
{
	QString name;
	QTextCharFormat charFormat;
	Qt::CaseSensitivity cs;
};

class QsvSyntaxHighlighter: public QSyntaxHighlighter
{
public:
	QsvSyntaxHighlighter( QsvColorDefFactory *colors, QsvLangDef *lang );
	QsvSyntaxHighlighter( QTextDocument *parent = nullptr, QsvColorDefFactory *colors=nullptr, QsvLangDef *lang=nullptr );
	QsvSyntaxHighlighter( QTextEdit *parent = nullptr, QsvColorDefFactory *colors=nullptr, QsvLangDef *lang=nullptr );
	QsvSyntaxHighlighter( QPlainTextEdit *parent = nullptr, QsvColorDefFactory *colors=nullptr, QsvLangDef *lang=nullptr );
	~QsvSyntaxHighlighter();
	void setHighlight( QsvLangDef *newLang=nullptr );
	void setColorsDef( QsvColorDefFactory *newColors=nullptr );

protected:
	void highlightBlock(const QString &text);

private:
	void addMapping( const QString mappingName, const QString &pattern, const QTextCharFormat &format, bool fullWord, Qt::CaseSensitivity cs);
	void addMappingFromName( const QString &pattern, const QString formatName, bool fullWord, Qt::CaseSensitivity cs );

	void drawText    ( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive );
	void drawRegExp  ( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive );
	void drawKeywords( QString text, QString s, QTextCharFormat &format, Qt::CaseSensitivity caseSensitive );

	QsvColorDefFactory	*colors;
	QsvLangDef		*language;
	QOrderedMap<QString,QsvLanguageEntity> mappings;
};
