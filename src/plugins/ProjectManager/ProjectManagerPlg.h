#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "iplugin.h"
#include <QList>
#include <QAbstractItemModel>

class QDockWidget;
class QTreeView;

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
	GenericItem* createChild();
	int row() const
	{
		return parentItem ?  parentItem->subChildren.indexOf(const_cast<GenericItem*>(this)) : 0 ;
	}
	virtual QString getDisplay(int column){ return name; Q_UNUSED(column); }
};

class GenericItemModel : public QAbstractItemModel
{
	Q_OBJECT
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

//////////////////////////////////////
// Project manager
//////////////////////////////////////
class ProjectManagerPlugin : public IPlugin
{
	Q_OBJECT
public:
	ProjectManagerPlugin();

	virtual void	showAbout();
	virtual void	on_client_merged( qmdiHost* host );
	virtual void	on_client_unmerged( qmdiHost* host );
public slots:
	// our code
	void onItemClicked(const QModelIndex &index);

private:
	QDockWidget *m_dockWidget;
	QTreeView *m_treeView;
	GenericItemModel *m_projectModel;
};

#endif // PROJECTMANAGER_H
