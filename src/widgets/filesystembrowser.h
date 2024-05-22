/**
 * \file filesystembrowser.h
 * \brief Definition of the system browser widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FileSystemBrowser
 */

#pragma once

#include <QStack>
#include <QWidget>

#include "ui_filesystembrowser.h"

class QString;
class QFileSystemModel;
class QCompleter;
class QTreeView;
class QListView;

class FileSystemBrowser : public QWidget, public Ui::FileSystemBrowser
{
	Q_OBJECT
public:
    FileSystemBrowser(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    QTreeView *getTreeView();
    QListView *getListView();
    QFileSystemModel *getDirModel();

private slots:
    void on_upButton_clicked();
    void on_backButton_clicked();
    void on_homeButton_clicked();
    void reloadButton_clicked();
    void on_forwardButton_clicked();
    void on_treeView_clicked(QModelIndex index);
    void on_filterEdit_textChanged(QString);

    void setRootPath(const QString &path = QString(), bool remember = true);
    void setRootIndex(const QModelIndex &index, bool remember = true);
    void init();

  private:
    void updateActions();

    QFileSystemModel *dirModel;
    QString currentPath;
    QStack<QString> history;
    QStack<QString> future;
    QCompleter *completer;
};
