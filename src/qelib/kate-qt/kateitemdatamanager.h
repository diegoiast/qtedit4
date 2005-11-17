#ifndef __KATE_ITEM_DATA_MANAGER__
#define __KATE_ITEM_DATA_MANAGER__

#include <QDomDocument>
#include <QString>
#include <QList>
#include "kateitemdata.h"

class kateItemDataManager
{
public:
	kateItemDataManager();
	kateItemDataManager( QDomDocument doc );
	kateItemDataManager( QString fileName );
	virtual ~kateItemDataManager();

	kateItemData getItemData( QString name );
	bool load( QDomDocument doc );
	bool load( QString fileName );
private:
	QList<kateItemData> itemDatas;
};

#endif // __KATE_ITEM_DATA_MANAGER__
