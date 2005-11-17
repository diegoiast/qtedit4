#include <QtDebug>
#include "katehighlight.h"

/* kate highlight */
kateHighlight::kateHighlight()
{
	
}

kateHighlight::kateHighlight( QDomDocument doc )
{
	load( doc );
}

bool kateHighlight::load( QDomDocument doc )
{
	uint i;

// load each list
//  TODO: should be
// 	QDomNodeList nodeList = doc.elementsByTagName("highlighting").item(0).childNodes().elementsByTagName("list");
	QDomNodeList nodeList = doc.elementsByTagName("list");
	for( i=0; i< nodeList.length(); i++ )
	{
		kateWordList l;
		if (!l.load(nodeList.item(i) ))
 			return false;
		qDebug() << "loading a new list" << l.name;
		list.append( l );
	}

	// load the contexts
	nodeList = doc.elementsByTagName("context");
	for( i=0; i< nodeList.length(); i++ )
	{
		kateContext c;
		if (!c.load(nodeList.item(i)))
			return false;
		contexts.append( c );
	}

	return true;
}

bool kateHighlight::save( QDomDocument doc )
{
// TODO
	return true;
}

