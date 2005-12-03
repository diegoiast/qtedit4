#ifndef __KATE_ITEM_DATA_H__
#define __KATE_ITEM_DATA_H__

#include <QDomNode>
#include <QColor>
#include <QTextCharFormat>

#include "kateqtglobal.h"


class kateItemData
{
public:
	kateItemData();
	kateItemData( QDomNode node );
	kateItemData( bool def );

	bool load( QDomNode node );
	bool save( QDomNode node );

	bool isBold();
	bool isUnderline();
	bool isItalic();
	QColor getColor();
	QColor getSelColor();
	QColor getBackground();
	QString getStyleNum();
	QTextCharFormat toCharFormat();
private:
	QStringMap attributes;
};

#endif // __KATE_ITEM_DATA_H__

