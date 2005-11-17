#ifndef __KATE_COMMENT_H__
#define __KATE_COMMENT_H__

#include <QString>
#include <QDomNode>
#include <QDomDocument>


class kateComment
{
public:
	kateComment();
	kateComment( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
private:	
	QString name;
	bool    isSingle;
	int     position;
	QString start;
	QString end;
	QString region;
};

#endif // __KATE_COMMENT_H__
