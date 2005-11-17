#include "katekeywords.h"

kateKeywords::kateKeywords()
{
}

kateKeywords::kateKeywords( QDomNode node )
{
	load( node );
}

bool kateKeywords::load( QDomNode node )
{
	caseSensitive       = isTrue( node.attributes().namedItem("casesensitive").nodeValue() );
	weakDelimiter       = node.attributes().namedItem("casesensitive").nodeValue();
	aditionalDelimiter  = node.attributes().namedItem("additionalDeliminator").nodeValue();
	wordWrapDeliminator = node.attributes().namedItem("wordWrapDeliminator").nodeValue();

	return true;
}

bool kateKeywords::save( QDomNode node )
{
	return true;
}

bool kateKeywords::isTrue( QString value )
{
	if (value == "1")
		return true;

	if (value == "true")
		return true;

	// else ....
	return false;
}
