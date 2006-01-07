#include <QtDebug>
#include "qelib/qtsourceview/qorderedmap.h"


 
void test_order_map()
{
	QOrderedMap	< QString, float > map;
	QOrderedMapNode	< QString, float > i;
//	QOrderedMap< int, float >::QOrderedMapNodeI i;

	map.add( "three", 3.3 );
	map.add( "five", 5.5 );
	map.add( "one", 1 );
	map.add( "two", 2.2 );
	map.add( "four", 4.4 );

	foreach(  i, map.keys() )
	{
		qDebug() << i.key << ":" << i.value;
	}
	qDebug() << map.value("three");
	qDebug() << map["five"];
}
