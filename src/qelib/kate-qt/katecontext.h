#ifndef __KATE_CONTEXT_H__
#define __KATE_CONTEXT_H__

#include <QDomNode>

#include "kateqtglobal.h"
#include "katehighlightrule.h"


class kateContext
{
public:
	kateContext();
	kateContext( QDomNode node );
	bool load( QDomNode node );
	bool save( QDomNode node );
private:
	QStringMap attributes;
	QList<kateHighlightRule> rules;
};

#endif // __KATE_CONTEXT_H__
