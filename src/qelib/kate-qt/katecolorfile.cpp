#include <QDomDocument>
#include <QFile>
#include "katecolorfile.h"

kateColorFile::kateColorFile()
{
}

kateColorFile::kateColorFile( QString fileName )
{
	loadFromFile( fileName );
}


bool kateColorFile::loadFromFile( QString fileName )
{
	QDomDocument doc("itemDataDoc");
	QFile file(fileName);
	QString s;

	if (!file.open(QIODevice::ReadOnly))
		return false;

	if (!doc.setContent(&file))
	{
		file.close();
		return false;
	}
	file.close();

	return load( doc );
}

bool kateColorFile::saveToFile( QString fileName )
{
	QDomDocument doc("language");
	QFile file(fileName);
	QString s;
	bool status;

	if (!file.open(QIODevice::ReadWrite))
		return false;

	status = save( doc );
	file.close();

	return status;
}


bool kateColorFile::load( QDomDocument doc )
{
	/* load the attributes of this language */
	QDomNodeList list = doc.elementsByTagName("itemDatas");
// 	QDomNode n;
// 	foreach( list, n )

	for( uint n=0 ;n<list.length(); n++ )
	{
		kateItemData item;
		if (item.load(list.item(n)))
			itemDatas.append( item );
	}

	return true;
}


bool kateColorFile::save( QDomDocument doc )
{
}


kateItemData kateColorFile::getItemData( QString name )
{
	//QList<kateItemData> itemDatas;
	kateItemData d;
	foreach( d, itemDatas)
	{
	}
}
