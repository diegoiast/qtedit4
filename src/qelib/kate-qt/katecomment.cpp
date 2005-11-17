#include "katecomment.h"

kateComment::kateComment()
{
}

kateComment::kateComment( QDomNode node )
{
	load( node );
}

bool kateComment::load( QDomNode node )
{
/*	QString name;
	bool    isSingle;
	int     position;
	QString start;
	QString end;
	QString region;*/

	QDomNodeList list = node.childNodes();
	int i = 0;
	int listCount = list.length();
	
	for( ;i<listCount; i++ )
	{
		QDomNode n = list.item(i);

		if (!n.hasAttributes())
			return false;
		
		QString name = n.attributes().namedItem("name").nodeValue();
		
		if (name == "singleLine")
		{
			isSingle = true;

			start    = n.attributes().namedItem("start").nodeValue();
			if (n.attributes().contains("position") )
				position = n.attributes().namedItem("position").nodeValue().toInt(); // optional
			else
				position = 0;

			end      = "";
			region   = "";
		}
		else	if (name == "multiLine")
			{
				isSingle = false;

				start    = n.attributes().namedItem("start").nodeValue();
				end      = n.attributes().namedItem("end").nodeValue();
				region   = n.attributes().namedItem("region").nodeValue();
				
				position = 0;
			}
			else
				return false;
		
	}

	return true;
}


bool kateComment::save( QDomNode node )
{
	return true;
}
