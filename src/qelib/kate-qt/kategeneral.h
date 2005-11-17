#ifndef __KATE_GENERAL_H__
#define __KATE_GENERAL_H__

#include <QDomDocument>

#include "katekeywords.h"
#include "katecomment.h"
#include "katefolding.h"


class kateGeneral
{
public:
	kateGeneral();
	kateGeneral( QDomDocument doc );
	bool load( QDomDocument doc );
	bool save( QDomDocument doc );
private:
	kateKeywords keywords;
	kateComment  comment;
	kateFolding  folding;
};

#endif // __KATE_GENERAL_H__
