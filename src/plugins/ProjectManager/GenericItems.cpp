#include "GenericItems.h"
#include <QDir>
#include <QElapsedTimer>
#include <QTime>

DirectoryModel::DirectoryModel(QObject *parent) : QAbstractTableModel(parent) {}

int DirectoryModel::rowCount(const QModelIndex &) const { return fileList.count(); }

int DirectoryModel::columnCount(const QModelIndex &) const { return 1; }

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

void DirectoryModel::removeAllDirs() {
    beginResetModel();
    directoryList.clear();
    fileList.clear();
    endResetModel();
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

bool FilenameMatches(const QString &fileName, const QString &goodList, const QString &badList) {
    if (!badList.isEmpty()) {
        auto list = badList.split(";");
        for (const auto &rule : list) {
            if (rule.length() < 3) {
                continue;
            }
            auto clean_rule = rule.trimmed();
            if (clean_rule.isEmpty()) {
                continue;
            }
            auto options = QRegularExpression::UnanchoredWildcardConversion;
            auto pattern = QRegularExpression::wildcardToRegularExpression(rule, options);
            auto escapedFile = QRegularExpression::escape(fileName);
            auto regex = QRegularExpression(pattern);
            bool matches = regex.match(escapedFile).hasMatch();
            if (matches) {
                return false;
            }
        }
    }

    bool filterMatchFound = true;
    if (!goodList.isEmpty()) {
        filterMatchFound = false;
        auto list = goodList.split(";");
        for (const auto &rule : list) {
            auto clean_rule = rule.trimmed();
            if (clean_rule.isEmpty()) {
                continue;
            }
            auto options = QRegularExpression::UnanchoredWildcardConversion;
            auto pattern = QRegularExpression::wildcardToRegularExpression(rule, options);
            auto escapedFile = QRegularExpression::escape(fileName);
            auto regex = QRegularExpression(pattern);
            bool matches = regex.match(escapedFile).hasMatch();
            if (matches) {
                filterMatchFound = true;
                break;
            }
        }
    }
    return filterMatchFound;
}
