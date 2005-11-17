#include <QtDebug>
#include "katewordlist.h"

kateWordList::kateWordList()
{
}

kateWordList::kateWordList( QDomNode node )
{
	load( node );
}

bool kateWordList::load( QDomNode node )
{
// 	qDebug("kateWordList::load()");
	// every list must have a name 
	if (!node.hasAttributes() )
		return false;
		
	if (!node.attributes().contains("name"))
		return false;
	
	// get the name of the list 
	name = node.attributes().namedItem("name").nodeValue();
	
	// get the itmes on that list 
	QDomNodeList nodeList = node.childNodes();
	int i = nodeList.length();
	for ( ; i!=0; i-- )
		items.insert( 0, nodeList.item(i).nodeValue() );

	return true;
}

bool kateWordList::save( QDomNode node )
{
// TODO
	return false;
}
