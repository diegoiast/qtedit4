#include <qemymap.h>

QEMyMapNode::QEMyMapNode( QString k, QTextCharFormat v )
{
	key = k;
	value = v;
}

QEMyMap::QEMyMap()
{
	// construst the object... yey!!!
}

QEMyMap::~QEMyMap()
{
	// destruct the object... yey!!!
}

void QEMyMap::add( QString k, QTextCharFormat v )
{
	if (contains(k))
		remove( k );
		
	QEMyMapNode node( k, v );
	nodes.append( node );
}

void QEMyMap::remove( QString k )
{
	int i = 0;
	foreach( QEMyMapNode n, nodes )
	{
		i ++;
		if (n.key == k )
		{
			//nodes.erase( n );
			nodes.removeAt( i );
			return;
		}
	}
}

void QEMyMap::clear()
{
	nodes.clear();
}

bool QEMyMap::contains( QString key )
{
	foreach( QEMyMapNode n, nodes )
	{
		if (n.key == key )
			return true;
	}
	
	return false;
}

int QEMyMap::count()
{
	return nodes.count();
}

bool QEMyMap::empty()
{
	return nodes.empty();
}


QList<QEMyMapNode> QEMyMap::keys()
{
	return nodes;
}
