#include "GenericItems.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QRunnable>
#include <QThreadPool>
#include <QTime>

FilesWorker::FilesWorker(const QString &rootPath, QObject *parent)
    : QObject{parent}, _rootPath(rootPath) {}

void FilesWorker::run() {
    QStringList files;
    QDirIterator i(_rootPath, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);

    emit started(_rootPath);
    while (i.hasNext()) {
        i.next();
        files << i.fileInfo().absoluteFilePath();

        // batch using any criteria
        if (files.size() == 1000) {
            emit filesLoaded(files);
            files.clear();
            QApplication::processEvents();
        }
    }
    if (!files.empty()) {
        emit filesLoaded(files);
    }
    emit finished();
}

DirectoryModel::DirectoryModel(QObject *parent) : QAbstractTableModel(parent) {}

int DirectoryModel::rowCount(const QModelIndex &) const { return fileList.count(); }

int DirectoryModel::columnCount(const QModelIndex &) const { return 1; }

QVariant DirectoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return displayForItem(index.row());
    case Qt::UserRole:
        return fileList.at(index.row());
    }
    return QVariant();
}

QString DirectoryModel::displayForItem(size_t i) const {
    auto fileName = fileList.at(i);
    auto path = [fileName, this]() {
        for (auto const &d : directoryList) {
            auto fixedFileName = fileName;
            auto fixedDirName = d;
            if (fixedFileName.startsWith(fixedDirName)) {
                return fixedDirName;
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

    FilesWorker *loader = new FilesWorker(path);
    connect(loader, &FilesWorker::started, this, &DirectoryModel::scanStarted,
            Qt::QueuedConnection);
    connect(loader, &FilesWorker::filesLoaded, this, &DirectoryModel::newFiles,
            Qt::QueuedConnection);
    connect(loader, &FilesWorker::finished, this, &DirectoryModel::scanFinished,
            Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(loader);
}

void DirectoryModel::scanStarted(const QString &rootPath) {
    directoryList.push_back(QDir::toNativeSeparators(rootPath));
}

void DirectoryModel::newFiles(const QStringList &files) {
    int startRow = fileList.size();
    int endRow = startRow + files.size() - 1;
    if (fileList.isSharedWith(files)) {
        return;
    }
    beginInsertRows(QModelIndex(), startRow, endRow);
    for (auto const &f : files) {
        fileList.push_back(QDir::toNativeSeparators(f));
    }
    endInsertRows();
    QApplication::processEvents();
}

void DirectoryModel::scanFinished() {
    emit didFinishLoading();
    qInfo() << "DirectoryModel: " << fileList.size() << "files loaded for" << directoryList;
}

void DirectoryModel::removeDirectory(const QString &path) {
    if (directoryList.contains(path)) {
        return;
    }
    beginResetModel();
    directoryList.removeAll(path);
    QDir dir(path);
    removeDirectoryImpl(dir);
    endResetModel();
}

void DirectoryModel::removeDirectoryImpl(const QDir &dir) {
    auto list = dir.entryInfoList();
    for (const auto &fi : std::as_const(list)) {
        if (fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }

        if (fi.isDir()) {
            removeDirectoryImpl(fi.absoluteFilePath());
        } else {
            fileList.removeAll(fi.absoluteFilePath());
        }
    }
}

bool FilenameMatches(const QString &fileName, const QString &goodList, const QString &badList) {
    if (!badList.isEmpty()) {
        auto list = badList.split(";");
        for (const auto &rule : std::as_const(list)) {
            if (rule.length() < 3) {
                continue;
            }
            auto clean_rule = rule.trimmed();
            if (clean_rule.isEmpty()) {
                continue;
            }
            auto options = QRegularExpression::UnanchoredWildcardConversion;
            auto pattern = QRegularExpression::wildcardToRegularExpression(rule, options);
            auto regex = QRegularExpression(pattern);
            auto matches = regex.match(fileName).hasMatch();
            if (matches) {
                return false;
            }
        }
    }

    bool filterMatchFound = true;
    if (!goodList.isEmpty()) {
        filterMatchFound = false;
        auto list = goodList.split(";");
        for (const auto &rule : std::as_const(list)) {
            auto clean_rule = rule.trimmed();
            if (clean_rule.isEmpty()) {
                continue;
            }
            auto options = QRegularExpression::UnanchoredWildcardConversion;
            auto pattern = QRegularExpression::wildcardToRegularExpression(rule, options);
            auto regex = QRegularExpression(pattern);
            auto matches = regex.match(fileName).hasMatch();
            if (matches) {
                filterMatchFound = true;
                break;
            }
        }
    }
    return filterMatchFound;
}

FilterOutProxyModel::FilterOutProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {}

void FilterOutProxyModel::setFilterWildcards(const QString &wildcards) {
    m_filterWildcards = wildcards;
    invalidateFilter();
}

void FilterOutProxyModel::setFilterOutWildcard(const QString &wildcard) {
    m_filterOutWildcard = wildcard;
    invalidateFilter();
}

bool FilterOutProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QString filePath = sourceModel()->data(index, Qt::DisplayRole).toString();
    return FilenameMatches(filePath, m_filterWildcards, m_filterOutWildcard);
}

bool FilterOutProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    auto leftData = sourceModel()->data(left).toString();
    auto rightData = sourceModel()->data(right).toString();
    auto l = countOccurrences(leftData, "/\\");
    auto r = countOccurrences(rightData, "/\\");

    if (l < r) {
        return true;
    }
    if (l > r) {
        return false;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

int FilterOutProxyModel::countOccurrences(const QString &str, const QString &targets) {
    auto count = 0;
    for (auto &ch : str) {
        if (targets.contains(ch)) {
            count++;
        }
    }
    return count;
}
