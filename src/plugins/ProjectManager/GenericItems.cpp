#include "GenericItems.h"
#include <QDir>
#include <QElapsedTimer>
#include <QTime>

DirectoryModel::DirectoryModel(QObject *parent) : QAbstractTableModel(parent) {}

int DirectoryModel::rowCount(const QModelIndex &parent) const { return fileList.count(); }

int DirectoryModel::columnCount(const QModelIndex &parent) const { return 1; }

QVariant DirectoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return displayForItem(index.row());
}

QString DirectoryModel::displayForItem(size_t i) const {
    auto fileName = fileList.at(i);
    auto path = [fileName, this]() {
        for (auto d : directoryList) {
            if (fileName.startsWith(d)) {
                return d;
            }
        }
        return QString{};
    }();

    auto l = path.size();
    if (!path.endsWith('/') && !path.endsWith('\\')) {
        l++;
    }
    return fileName.remove(0, l);
}

QString DirectoryModel::fileNameForItem(size_t i) const {
    auto s = displayForItem(i);
    auto l = s.lastIndexOf('/');
    if (l == -1) {
        l = s.lastIndexOf('\\');
    }
    if (l != -1) {
        s = s.right(l + 1);
    }
    return s;
}

void DirectoryModel::addDirectory(const QString &path) {
    if (directoryList.contains(path)) {
        return;
    }
    beginResetModel();
    directoryList.append(path);
    QDir dir(path);
    addDirectoryImpl(dir);
    endResetModel();
}

void DirectoryModel::removeDirectory(const QString &directory) {}

void DirectoryModel::addDirectoryImpl(const QDir &dir) {
    auto list = dir.entryInfoList();
    for (auto fi : list) {
        if (fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }

        if (fi.isDir()) {
            addDirectoryImpl(fi.absoluteFilePath());
        } else {
            fileList.append(fi.absoluteFilePath());
        }
    }
}

void DirectoryModel::removeDirectoryImpl(const QDir &directory) {}
