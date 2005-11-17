#ifndef __qe__MY_MAP__

#include <QString>
#include <QList>
#include <QTextCharFormat>

class QEMyMapNode
{
public:
	QEMyMapNode( QString k, QTextCharFormat v );
	QString key;
	QTextCharFormat value;
};


class QEMyMap
{
public:
	QEMyMap();
	~QEMyMap();
	
	void add( QString k, QTextCharFormat v );
	void remove( QString k );
	
	void clear();
	bool contains( QString key );
	int count();
	bool empty();
	QList<QEMyMapNode> keys();
	
	
private:
	QList<QEMyMapNode> nodes;
};

#endif
