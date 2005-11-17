#include "katecontext.h"

/**
 * \class kateContext 
 * \brief contexts in kate are section of text which should be painted in the same color
 */


kateContext::kateContext()
{
}

kateContext::kateContext( QDomNode node )
{
	load( node );
}

bool kateContext::load( QDomNode node )
{
	// load the attributes
	for( uint i=0; i<node.attributes().count(); i++ )
	{
		attributes[node.attributes().item(i).nodeName()] =
		   node.attributes().item(i).nodeValue();
	}

	QDomNodeList nodeList = node.childNodes();
	int i = nodeList.length();
	for ( ; i!=0; i-- )
		rules.insert( 0, kateHighlightRule(nodeList.item(i)) );

//	QList<kateHighlightRule> rules;
	return true;
}

bool kateContext::save( QDomNode node )
{
/*	QStringMap attributes;
	QList<kateHighlightRule> rules;*/
	return true;
}
