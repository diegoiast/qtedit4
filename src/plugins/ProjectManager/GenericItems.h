#ifndef GENERICITEMS_H
#define GENERICITEMS_H

#include <QAbstractItemModel>
#include <QString>
#include <QList>
#include <QCompleter>

//////////////////////////////////////
// generic tree view
//////////////////////////////////////
struct GenericItem {
	GenericItem(const QString &n, GenericItem *parent=NULL)
	{
		parentItem = parent;
		name = n;
	}

	GenericItem(GenericItem *parent=NULL)
	{
		parentItem = parent;
	}

	GenericItem *parentItem;
	QString name;
	QList<GenericItem*> subChildren;
	int row() const
	{
		return parentItem ?  parentItem->subChildren.indexOf(const_cast<GenericItem*>(this)) : 0 ;
	}
	virtual QString getDisplay(int column) const { return name; Q_UNUSED(column); }
};

class GenericItemModel : public QAbstractItemModel
{
protected:
	GenericItem *rootItem;

public:
	GenericItemModel(QObject *parent = 0);
	~GenericItemModel();

	// re-implementation of the interface
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const;
	virtual QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	// our new code
	virtual GenericItem* addItem( GenericItem* newItem, GenericItem *parent=NULL);

	const GenericItem* getGenericRootItem(){return rootItem;}
};

class GenericItemCompleter: public QCompleter{
	QString pathFromIndex(const QModelIndex &index) const;
};

class FileItem : public GenericItem {
public:
	bool isDirectory;
	QString fullPath;
	FileItem(const QString &path, GenericItem *parent=NULL, bool isDir=false);
};

class FoldersModel: public GenericItemModel {
public:
	FoldersModel(QObject *parent = 0): GenericItemModel(parent){}
	int processDir(const QString &path);
	int processDir(const QString &path, GenericItem *parent);
};


#endif // GENERICITEMS_H
