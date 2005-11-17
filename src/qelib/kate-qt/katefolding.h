#ifndef __KATE_FOLDING_H__
#define __KATE_FOLDING_H__

#include <QDomDocument>


class kateFolding
{
public:
	kateFolding();
	kateFolding( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
private:
	bool indintationSensitive;
};

#endif // __KATE_FOLDING_H__
