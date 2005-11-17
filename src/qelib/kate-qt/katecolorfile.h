#ifndef __KATE_COLOR_FILE_H__
#define __KATE_COLOR_FILE_H__

#include <QString>
#include <QDomDocument>
#include "katehighlight.h"

class kateColorFile
{
public:
	kateColorFile();
	kateColorFile( QString fileName );

	bool loadFromFile( QString fileName );
	bool saveToFile( QString fileName );

	bool load( QDomDocument doc );
	bool save( QDomDocument doc );

	kateItemData getItemData( QString name );
private:
	QList<kateItemData> itemDatas;
};

#endif // __KATE_COLOR_FILE_H__
