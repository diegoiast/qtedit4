#include "GenericItems.h"
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
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
    return QDir::toNativeSeparators(fileName.remove(0, l));
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
    qInfo() << "loading" << rootPath;
    directoryList << rootPath;
}

void DirectoryModel::newFiles(const QStringList &files) {
    int startRow = fileList.size();
    int endRow = startRow + files.size() - 1;
    if (fileList.isSharedWith(files)) {
        return;
    }
    beginInsertRows(QModelIndex(), startRow, endRow);
    fileList << files;
    endInsertRows();
    qInfo() << "Inserted" << files.size() << "files. First file:" << files.first();
}

void DirectoryModel::scanFinished() { qInfo() << fileList.size() << "files loaded"; }

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
        for (const auto &rule : std::as_const(list)) {
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
