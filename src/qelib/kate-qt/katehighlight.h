#ifndef __KATE_HIGHLIGHT__
#define __KATE_HIGHLIGHT__

#include <QDomDocument>
#include <QList>

#include "kateqtglobal.h"
#include "katewordlist.h"
#include "katecontext.h"
#include "kateitemdata.h"


class kateHighlight
{
public:
	kateHighlight();
	kateHighlight( QDomDocument doc );
	bool load( QDomDocument doc );
	bool save( QDomDocument doc );
private:
	QStringMap          attributes;
	QList<kateWordList> list;
	QList<kateContext>  contexts;
	QList<kateItemData> itemDatas;
};

#endif // __KATE_HIGHLIGHT__
