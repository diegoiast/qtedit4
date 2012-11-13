#include <QDir>
#include <QTime>
#include "GenericItems.h"

GenericItemModel::GenericItemModel(QObject *parent)
	:QAbstractItemModel(parent)
{
	rootItem =  new GenericItem;
}

GenericItemModel::~GenericItemModel()
{
}

QVariant GenericItemModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	GenericItem *item = static_cast<GenericItem*>(index.internalPointer());
	return item->getDisplay(index.column());
}

Qt::ItemFlags GenericItemModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant GenericItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Name");
		default:
			return QVariant();
		}
	}
	return QVariant();
}

QModelIndex GenericItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	GenericItem *parentItem;

	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<GenericItem*>(parent.internalPointer());

	GenericItem *childItem = parentItem->subChildren.value(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex GenericItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	GenericItem *childItem = static_cast<GenericItem*>(index.internalPointer());
	GenericItem *parentItem = childItem->parentItem;

	if (parentItem == rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int GenericItemModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	GenericItem *parentItem;
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<GenericItem*>(parent.internalPointer());

	return parentItem->subChildren.count();
}

int GenericItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

GenericItem* GenericItemModel::addItem(GenericItem *newItem, GenericItem *parent)
{
	if (parent == NULL)
		parent = rootItem;

	beginResetModel();
	parent->subChildren.append(newItem);
	newItem->parentItem = parent;
	endResetModel();

	return newItem;
}

QString GenericItemCompleter::pathFromIndex(const QModelIndex &index) const
{
	QString s;
	GenericItem *item = static_cast<GenericItem*>(index.internalPointer());
	while (item!=NULL) {
		s.prepend('/');
		s.prepend(item->getDisplay(0));

		item = item->parentItem;
	}
	return s;
}

FileItem::FileItem(const QString &path, GenericItem *parent, bool isDir)
{
	parentItem = parent;
	fullPath = path;
	isDirectory = isDir;

	int i=fullPath.lastIndexOf(QDir::separator());
	int k=fullPath.length();
	if (i==-1)
		i=fullPath.lastIndexOf('/');
	if (i==k-1)
		i=fullPath.lastIndexOf(QDir::separator(),i-1);
	if (i == -1)
		name = fullPath;
	else
		name = fullPath.mid(i+1);
	if (name.endsWith('/') || name.endsWith(QDir::separator()))
		name.chop(1);
}

int FoldersModel::processDir(const QString &path)
{
	QTime t;
	t.start();
	int i = processDir(path, (FileItem*)addItem(new FileItem(path,NULL,true),rootItem));
	qDebug("Loading took %d msec (loaded %d items)", t.elapsed(), i);
	return i;
}

int FoldersModel::processDir(const QString &path,  GenericItem *parent)
{
	QDir dir(path);
	QFileInfoList list = dir.entryInfoList();
	int count = 0;

	FileItem *item;
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		QString s;

//		s = fileInfo.absoluteFilePath();
//		if (s == path)
//			continue;
		s = fileInfo.fileName();
		if (s==".." || s==".")
			continue;
		s = fileInfo.absoluteFilePath();
		if (s == path)
			continue;
		item = (FileItem*)addItem(new FileItem(s),parent);
		count++;
		if (fileInfo.isDir()) {
			item->isDirectory = true;
			count += processDir(s,item);
		}
	}

	return count;
}
