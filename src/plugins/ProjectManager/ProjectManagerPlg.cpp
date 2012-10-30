#include "ProjectManagerPlg.h"
#include "qmdiserver.h"
#include "qmdihost.h"
#include <QMessageBox>
#include <QMainWindow>
#include <QDockWidget>
#include <QTreeView>
#include <QDir>
#include <QTime>

//////////////////////////////////////
// generic tree view
//////////////////////////////////////

GenericItemModel::GenericItemModel(QObject *parent)
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

	if (role != Qt::DisplayRole)
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

//////////////////////////////////////
// Project manager
//////////////////////////////////////

ProjectManagerPlugin::ProjectManagerPlugin()
{
	name = tr("Project manager");
	author = tr("Diego Iastrubni <elcuco@kde.org>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = true;

	m_dockWidget = NULL;
	m_treeView = NULL;
	m_projectModel = NULL;
}

void ProjectManagerPlugin::showAbout()
{
    QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "The project manager plugin" );
}

void ProjectManagerPlugin::onClientMerged(qmdiHost *host)
{
	if (m_dockWidget)
		return;

	QMainWindow *window = dynamic_cast<QMainWindow*>(host);
	m_dockWidget = new QDockWidget(window);
	m_dockWidget->setObjectName("m_dockWidget");
	m_dockWidget->setWindowTitle( tr("Project") );
	m_treeView = new QTreeView(m_dockWidget);
	m_dockWidget->setWidget(m_treeView);
#if 0
	m_projectModel = new GenericItemModel(m_treeView);
	GenericItem *item;
	item = m_projectModel->addItem(new GenericItem("item1"));
	item = m_projectModel->addItem(new GenericItem("item2"));
	item = m_projectModel->addItem(new GenericItem("item3"));
	m_projectModel->addItem(new GenericItem("sub-item3-1"), item);
	m_projectModel->addItem(new GenericItem("sub-item3-2"), item);
	m_projectModel->addItem(new GenericItem("sub-item3-3"), item);
#else
	m_projectModel = new FoldersModel(m_treeView);
	//((FoldersModel*)(m_projectModel))->processDir("/home/elcuco/src/qtedit4/");
	((FoldersModel*)(m_projectModel))->processDir("/home/elcuco/src/qt-creator/");
#endif
	m_treeView->setModel(m_projectModel);
	window->addDockWidget( Qt::LeftDockWidgetArea, m_dockWidget );
}

void ProjectManagerPlugin::onClientUnmerged(qmdiHost *host)
{
	delete(m_dockWidget);
	m_dockWidget = NULL;
	Q_UNUSED( host );
}

FileItem::FileItem(const QString &path, GenericItem *parent)
{
	parentItem = parent;
	fullPath = path;

	int i=fullPath.lastIndexOf(QDir::separator());
	if (i==fullPath.length())
		fullPath.lastIndexOf(QDir::separator(),i-1);
	if (i == -1)
		name = fullPath;
	else
		name = fullPath.mid(i+1);
}

int FoldersModel::processDir(const QString &path)
{
	QTime t;
	t.start();
	int i = processDir(path, rootItem);
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
			count += processDir(s,item);
		}
	}

	return count;
}
