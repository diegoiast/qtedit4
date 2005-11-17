#include <QtDebug>
#include <QFile>
#include "kateitemdatamanager.h"

kateItemDataManager::kateItemDataManager()
{
}

kateItemDataManager::kateItemDataManager( QDomDocument doc )
{
	load( doc );
}

kateItemDataManager::kateItemDataManager( QString fileName )
{
	load( fileName );
}

kateItemDataManager::~kateItemDataManager()
{
}

kateItemData kateItemDataManager::getItemData( QString name )
{
	foreach(kateItemData item, itemDatas)
	{	
		if (item.getStyleNum() == name )
			return item;
	}

	// new empthy one
// 	qDebug( "kateItemDataManager::getItemData() - could not find an item data" );
	return kateItemData();
}

bool kateItemDataManager::load( QDomDocument doc )
{
	/* load the attributes of this language */
	QDomNodeList list = doc.elementsByTagName("itemData");

// 	qDebug( "kateItemDataManager::load(QDomDocument) - Found %i elements", list.length());
	for( uint n=0 ;n<list.length(); n++ )
	{
		kateItemData item;
		if (item.load(list.item(n)))
			itemDatas.append( item );
	}

	return true;
}

bool kateItemDataManager::load( QString fileName )
{
	QDomDocument doc("language");
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
