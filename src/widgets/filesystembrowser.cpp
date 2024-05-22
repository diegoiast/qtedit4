/**
 * \file filesystembrowser.cpp
 * \brief Implementation of the system browser widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FileSystemBrowser
 */

#include <QCompleter>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QTimer>

#include "filesystembrowser.h"

FileSystemBrowser::FileSystemBrowser(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {
    setupUi(this);
    this->backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    this->forwardButton->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    this->homeButton->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    this->upButton->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    dirModel = new QFileSystemModel(this);
    dirModel->setRootPath(QDir::rootPath());
    dirModel->setReadOnly(false);

    listView->setModel(dirModel);
    treeView->setModel(dirModel);
    treeView->setDragEnabled(true);
    treeView->setAcceptDrops(true);

    listView->setModel(dirModel);
    treeView->setModel(dirModel);
    treeView->setDragEnabled(true);
    treeView->setAcceptDrops(true);

    completer = new QCompleter(locationLineEdit);
    completer->setModel(dirModel);
    locationLineEdit->setCompleter(completer);

    connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(setRootIndex(QModelIndex)));
    connect(locationLineEdit, SIGNAL(returnPressed()), this, SLOT(setRootPath()));
    connect(listView, &QAbstractItemView::clicked, this, &FileSystemBrowser::on_treeView_clicked);

    QList<int> l;
    l << 0 << splitter->height();
    splitter->setSizes(l);
    QTimer::singleShot(0, this, SLOT(init()));
}

QTreeView *FileSystemBrowser::getTreeView() { return treeView; }

QListView *FileSystemBrowser::getListView() { return listView; }

QFileSystemModel *FileSystemBrowser::getDirModel() { return dirModel; }

void FileSystemBrowser::on_treeView_clicked(QModelIndex index) {
    if (dirModel->fileInfo(index).isDir()) {
        setRootIndex(index);
    }
}

void FileSystemBrowser::on_backButton_clicked() {
    future.push(currentPath);
    //    currentPath = m_hist();
    setRootPath(currentPath, false);
}

void FileSystemBrowser::on_homeButton_clicked() { setRootPath(QDir::homePath()); }

void FileSystemBrowser::reloadButton_clicked() {}

void FileSystemBrowser::on_forwardButton_clicked() {
    history.push(currentPath);
    currentPath = future.pop();
    setRootPath(currentPath, false);
}

void FileSystemBrowser::on_upButton_clicked() { setRootIndex(listView->rootIndex().parent()); }

void FileSystemBrowser::on_filterEdit_textChanged(QString s) {
    s = s.simplified();
    if (s.isEmpty()) {
        filterEdit->setText("*.*");
    } else {
        dirModel->setNameFilters(s.split(';'));
    }
}

void FileSystemBrowser::setRootPath(const QString &path, bool remember) {
    if (path.isNull()) {
        setRootIndex(dirModel->index(locationLineEdit->text()), remember);
    } else {
        setRootIndex(dirModel->index(path), remember);
    }
}

void FileSystemBrowser::setRootIndex(const QModelIndex &index, bool remember) {
    QModelIndex dir = index.sibling(index.row(), 0);

    if (!dirModel->isDir(dir)) {
        dir = dir.parent();
    }

    if (dir != treeView->rootIndex() && dirModel->isDir(dir)) {
        if (remember) {
            future.clear();
            history.push(currentPath);
            currentPath = dirModel->filePath(dir);
        }
        listView->setRootIndex(dir);
        treeView->setCurrentIndex(index);
        locationLineEdit->setText(dirModel->filePath(dir));
    }

    updateActions();
}

void FileSystemBrowser::init() {
    currentPath = QDir::homePath();
    setRootPath(currentPath, false);
}

void FileSystemBrowser::updateActions() {
    backButton->setEnabled(!history.isEmpty());
    forwardButton->setEnabled(!future.isEmpty());
    // 	upButton->setEnabled(m_dirModel->fileInfo(treeView->rootIndex()).isRoot());
}

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 8; mixedindent off; indent-mode
// cstyle; kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix kate:
// show-tabs on;
