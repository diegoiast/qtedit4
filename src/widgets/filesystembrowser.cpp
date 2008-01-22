/**
 * \file filesystembrowser.cpp
 * \brief Implementation of the system browser widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FileSystemBrowser
 */

#include <QCompleter>
#include <QDirModel>
#include <QTimer>
#include "filesystembrowser.h"

FileSystemBrowser::FileSystemBrowser( QWidget * parent, Qt::WFlags f) 
	: QWidget(parent, f)
{
	setupUi(this);
	
	m_dirModel = new QDirModel(this);
	m_dirModel->setReadOnly(false);

	listView->setModel(m_dirModel);
	treeView->setModel(m_dirModel);
	treeView->setDragEnabled(true);
	treeView->setAcceptDrops(true);

	m_completer = new QCompleter(locationLineEdit);
	m_completer->setModel(m_dirModel);
	locationLineEdit->setCompleter(m_completer);

	connect(m_completer, SIGNAL(activated(QModelIndex)), this, SLOT(setRootIndex(QModelIndex)));
	connect(locationLineEdit,  SIGNAL(returnPressed()),        this, SLOT(setRootPath()));
// 	connect(treeView, SIGNAL(clicked(QModelIndex)),       this, SLOT(setRootIndex(QModelIndex)));
  	connect(listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(setRootIndex(QModelIndex)));
// 	QTimer::singleShot( 1000, this, SLOT(init()));
	m_currentPath = QDir::homePath();
	setRootPath(m_currentPath, false);
}

void FileSystemBrowser::on_treeView_clicked(QModelIndex index)
{
	setRootIndex(index);
}

void FileSystemBrowser::on_backButton_clicked()
{
	m_future.push(m_currentPath);
	m_currentPath = m_history.pop();
	setRootPath(m_currentPath, false);
}

void FileSystemBrowser::on_homeButton_clicked()
{
	setRootPath(QDir::homePath());
}

void FileSystemBrowser::on_reloadButton_clicked()
{
	m_dirModel->refresh( treeView->rootIndex() );
}

void FileSystemBrowser::on_forwardButton_clicked()
{
	m_history.push(m_currentPath);
	m_currentPath = m_future.pop();
	setRootPath(m_currentPath, false);
}

void FileSystemBrowser::on_upButton_clicked()
{
// 	setRootIndex(treeView->rootIndex().parent());
	setRootIndex(listView->rootIndex().parent());
}

void FileSystemBrowser::setRootPath(const QString& path, bool remember)
{
	if (path.isNull())
		setRootIndex(m_dirModel->index(locationLineEdit->text()), remember);
	else
		setRootIndex(m_dirModel->index(path), remember);
}

void FileSystemBrowser::setRootIndex(const QModelIndex& index, bool remember )
{
	QModelIndex dir = index.sibling(index.row(), 0);
	
	if (!m_dirModel->isDir(dir))
		dir = dir.parent();

	if (dir != treeView->rootIndex() && m_dirModel->isDir(dir))
	{
		if (remember)
		{
			m_future.clear();
			m_history.push(m_currentPath);
			m_currentPath = m_dirModel->filePath(dir);
		}
		listView->setRootIndex(dir);
		treeView->setCurrentIndex(index);
		locationLineEdit->setText(m_dirModel->filePath(dir));
	}

	updateActions();
}

void	FileSystemBrowser::init()
{
	m_currentPath = QDir::homePath();
	setRootPath(m_currentPath, false);
}

void	FileSystemBrowser::updateActions()
{
	backButton->setEnabled(!m_history.isEmpty());
	forwardButton->setEnabled(!m_future.isEmpty());
// 	upButton->setEnabled(m_dirModel->fileInfo(treeView->rootIndex()).isRoot());
}

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 8; mixedindent off; indent-mode cstyle;
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
