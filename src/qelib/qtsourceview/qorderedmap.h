#ifndef __QE_ORDERED_MAP_H__
#define __QE_ORDERED_MAP_H__

#include <QString>
#include <QList>
#include <QTextCharFormat>

template <class Key, class Value>
class QOrderedMapNode
{
public:
	Key	key;
	Value	value;
	QOrderedMapNode()
	{
	}

	QOrderedMapNode( Key k, Value v )
	{
		key  = k;
		value = v;
	}
};


/**
 * A string hash table. The twist, is that the objects are ordered inside
 * at the same order they were inserted (unlike QMap which orders by key or 
 * QHash which orders by arbitrarily order)
 */
template <class Key, class Value>
class QOrderedMap
{
public:
	QOrderedMap();
	~QOrderedMap();
	
	void add( Key k, Value v );
	void remove( Key k );
	
	void clear();
	bool contains( Key key );
	int count();
	bool empty();
	QList< QOrderedMapNode< Key, Value > > keys();
	Value operator[]( Key k );
	Value value( Key k );
	
private:
	QList< QOrderedMapNode< Key, Value > > nodes;
};


/// implementation
template <class Key, class Value>
QOrderedMap<Key, Value>::QOrderedMap()
{
	// construst the object... yey!!!
}

template <class Key, class Value>
QOrderedMap<Key, Value>::~QOrderedMap()
{
	// destroy the object... yey!!!
}

template <class Key, class Value>
void QOrderedMap<Key, Value>::add( Key k, Value v )
{
	if (contains(k))
		remove( k );
		
	QOrderedMapNode<Key, Value> node( k, v );
 	nodes.append( node );
}

template <class Key, class Value>
void QOrderedMap<Key, Value>::remove( Key k )
{
	int i = 0;
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
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

template <class Key, class Value>
void QOrderedMap<Key, Value>::clear()
{
	nodes.clear();
}

template <class Key, class Value>
bool QOrderedMap<Key, Value>::contains( Key key )
{
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
	{
		if (n.key == key )
			return true;
	}
	
	return false;
}

template <class Key, class Value>
int QOrderedMap<Key, Value>::count()
{
	return nodes.count();
}

template <class Key, class Value>
bool QOrderedMap<Key, Value>::empty()
{
 	return nodes.empty();
}

template <class Key, class Value>
QList< QOrderedMapNode< Key, Value > > QOrderedMap<Key, Value>::keys()
{
	return nodes;
}

template <class Key, class Value>
Value QOrderedMap<Key, Value>::operator[](Key k)
{
	return value( k );
}

template <class Key, class Value>
Value QOrderedMap<Key, Value>::value( Key k )
{
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
	{
		if (n.key == k )
			return n.value;
	}
}

#endif // __QE_ORDERED_MAP_H__
