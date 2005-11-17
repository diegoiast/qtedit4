#ifndef __KATE_KEYWORDS_H__
#define __KATE_KEYWORDS_H__

#include <QDomNode>
#include <QString>

class kateKeywords
{
public:
	kateKeywords();
	kateKeywords( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
	
	bool isTrue( QString value );
private:
	bool    caseSensitive;
	QString weakDelimiter;
	QString aditionalDelimiter;
	QString wordWrapDeliminator;
};

#endif // __KATE_KEYWORDS_H__
