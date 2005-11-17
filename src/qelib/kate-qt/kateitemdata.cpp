#include <QtDebug>
#include "kateitemdata.h"

/**
 * \class kateItemData
 * \brief and item data contains the color definition for a context
 */
kateItemData::kateItemData()
{
// 	attributes["color"] = "black";
// 	attributes["background"] = "white";
}

kateItemData::kateItemData( QDomNode node )
{
	load( node );
}

bool kateItemData::load( QDomNode node )
{
	uint attrCount = node.attributes().count();
	for( uint i=0; i< attrCount; i++ )
	{
		attributes[node.attributes().item(i).nodeName()] =
		   node.attributes().item(i).nodeValue();
	}

	return true;
}

bool kateItemData::save( QDomNode node )
{
//QStringMap attributes;
	return true;
}


bool kateItemData::isBold()
{
	if (!attributes.contains("bold"))
		return false;
	
	QString boldAttr = attributes["bold"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;
}

bool kateItemData::isUnderline()
{
	if (!attributes.contains("underline"))
		return false;
	
	QString boldAttr = attributes["underline"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;
}

bool kateItemData::isItalic()
{
	if (!attributes.contains("italic"))
		return false;
	
	QString boldAttr = attributes["italic"].toLower();
	if ((boldAttr == "1") || (boldAttr == "true") || (boldAttr == "yes"))
		return true;
	else
		return false;

}

QColor kateItemData::getColor()
{
	if (!attributes.contains("color"))
		return QColor();
	else 
		return QColor( attributes["color"] );
}

QColor kateItemData::getSelColor()
{
	if (!attributes.contains("selColor"))
		return QColor();
	else 
		return QColor( attributes["selColor"] );
}

QColor kateItemData::getBackground()
{
	if (!attributes.contains("background"))
		return QColor();
	else 
		return QColor( attributes["background"] );
}

QString kateItemData::getStyleNum()
{
// 	if (!attributes.contains("defStyleNum"))
// 		return QString();

	return attributes["defStyleNum"];
}

QTextCharFormat kateItemData::toCharFormat()
{
	QTextCharFormat f;

	if (attributes.contains("color"))
		f.setForeground( QColor(attributes["color"]) );

	if (attributes.contains("background"))
		f.setBackground( QColor(attributes["background"]) );

	if (isBold()) 
		f.setFontWeight(QFont::Bold);
	
	if (isItalic())
		f.setFontItalic( true );

	if (isUnderline())
		f.setFontUnderline( true );

	return f;
}
