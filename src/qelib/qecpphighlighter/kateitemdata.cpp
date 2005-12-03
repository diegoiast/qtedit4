#include <QtDebug>
#include "kateitemdata.h"

/**
 * \class kateItemData
 * \brief and item data contains the color definition for a context
 */
kateItemData::kateItemData()
{
}

kateItemData::kateItemData( QDomNode node )
{
	load( node );
}


/**
 * \brief constructor for emppty definitions
 * \param def use a default color?
 * 
 * If \p def is \p true a default color (black on white)
 * will be used for this item data.
 */
kateItemData::kateItemData( bool def )
{
	if (def)
	{
		attributes["color"] = "black";
		attributes["background"] = "white";
	}
}


/**
 * \brief Load the color definition from XML
 * \param node a DomNode to load the information frmo
 * \return true if succesfull
 * 
 * This function will create the item data from values found on
 * the XML. 
 * 
 * \see kateItemDataManager::load(QDomDocument)
 */
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
// TODO code this function
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


/**
 * \brief get a QTextCharFormat from this object
 * \return a well constructed QTextCharFormat which corresponds to this object
 * 
 * When applying the item-data to a QTextDocument, it is needed to convert
 * the item-data into a valid QTextCharFormat.
 */
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
