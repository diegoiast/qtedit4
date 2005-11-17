#ifndef __KATE_WORD_LIST_H__
#define __KATE_WORD_LIST_H__

#include <QDomNode>
#include <QStringList>

#include "kateqtglobal.h"


class kateWordList
{
public:
	kateWordList();
	kateWordList( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
// private:
	QString     name;
	QStringList items;
};

#endif // __KATE_WORD_LIST_H__
