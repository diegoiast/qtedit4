#include "katehighlightrule.h"

// this list must end with a NULL string
// also keep it in syc with the list on katehighlightrule.h
char *KateHighlightRuleNames[] =
{
	"DetectChar",
	"Detect2Chars",
	"AnyChar",
	"StringDetect",
	"RegExp",
	"keyword",
	"Int",
	"Float",
	"HICOct",
	"HlCStringChar",
	"HlCChar",
	"RangeDetect",
	"LineContinue",
	"IncludeRules",
	"DetectSpaces",
	"DetectIdentifier",
	0
};

kateHighlightRule::kateHighlightRule()
{
}

kateHighlightRule::kateHighlightRule( QDomNode node )
{
	load( node );
}

bool kateHighlightRule::load( QDomNode node )
{
	QDomNodeList list = node.childNodes();
	name = node.nodeName();
	type = getTypeFromName( name );

	for( uint i=0; i<list.item(0).attributes().count(); i++ )
	{
		attributes[list.item(0).attributes().item(i).nodeName()] =
		   list.item(0).attributes().item(i).nodeValue();
	}


	return true;
}

bool kateHighlightRule::save( QDomNode node )
{
/*	QString name;
	int type;
	QStringMap attributes;*/

	return true;
}

int kateHighlightRule::getTypeFromName( QString name )
{
	char **p;
	int  i = 0;
	
	for( p = KateHighlightRuleNames; *p != NULL; p++ )
	{
		if (name == *p)
			return i;
			
		i++;
	}
}
