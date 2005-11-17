#include "kategeneral.h"

kateGeneral::kateGeneral()
{
}

kateGeneral::kateGeneral( QDomDocument doc )
{
	load( doc );
}

bool kateGeneral::load( QDomDocument doc )
{
	keywords.load( doc.elementsByTagName("keywords").item(0) );
	comment.load( doc.elementsByTagName("comments").item(0) );
// 	folding.load( doc.elementsByTagName("folding").item(0)
	return true;
}

bool kateGeneral::save( QDomDocument doc )
{
/*	kateKeywords keywords;
	kateComment  comment;
	kateFolding  folding;*/
	return true;
}
