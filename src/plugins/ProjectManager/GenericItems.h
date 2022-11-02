#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QList>
#include <QCompleter>
#include <QDir>

class DirectoryModel : public QAbstractTableModel {
public:
    DirectoryModel(QObject *parent=NULL);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    const QString& getItem(size_t i) const { return fileList[i]; }
    QString displayForItem(size_t i) const;
    QString fileNameForItem(size_t i) const;

    void addDirectory(const QString& path);
    void removeDirectory(const QString& path);

private:
    void addDirectoryImpl(const QDir& directory);
    void removeDirectoryImpl(const QDir& directory);

    QStringList fileList;
    QStringList directoryList;
};
