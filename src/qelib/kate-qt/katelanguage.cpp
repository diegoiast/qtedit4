#include <QtDebug>
#include <QFile>
#include <QDomDocument>

#include "katelanguage.h"

kateLanguage::kateLanguage()
{
}

kateLanguage::kateLanguage( QString fileName )
{
	loadFromFile( fileName );
}

bool kateLanguage::loadFromFile( QString fileName )
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

bool kateLanguage::saveToFile( QString fileName )
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

bool kateLanguage::load( QDomDocument doc )
{
	/* load the attributes of this language */
	QDomNodeList list = doc.elementsByTagName("language");
	QDomNode n;
	
	//foreach( n, list.item(0).attributes() )
	for( uint i=0; i<list.item(0).attributes().count(); i++ )
	{
		n = list.item(0).attributes().item(i);
// 		qDebug() << "new attribute (" << i << ")" << n.nodeName();
		attributes[n.nodeName()] = n.nodeValue();
	}

	/* load the highlight */
	if (!highlight.load( doc ))
		return false;
	/* load general properties */
	if (!general.load( doc ))
		return false;

	return true;
}

bool kateLanguage::save( QDomDocument doc )
{
	// TODO
	return false;
}

