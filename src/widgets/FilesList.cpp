#include "FilesList.hpp"
#include "LoadingWidget.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QQueue>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

// Normalize path to use `/`
static inline QString normalizeFilePath(const QString &path) {
    return QDir::fromNativeSeparators(path);
}

static inline QString normalizeDirPath(const QString &path) {
    auto p = QDir::fromNativeSeparators(path);
    if (!p.endsWith('/')) {
        p = p + '/';
    }
    return p;
}

// Convert glob to regex list
static QList<QRegularExpression> toRegexList(const QStringList &patterns) {
    auto list = QList<QRegularExpression>();
    for (auto &pat : patterns) {
        auto rx = QString();
        for (auto ch : pat.trimmed()) {
            if (ch == '*') {
                rx += ".*";
            } else if (ch == '?') {
                rx += '.';
            } else if (QString("[](){}.+^$|\\").contains(ch)) {
                rx += '\\' + ch;
            } else {
                rx += ch;
            }
        }
        if (!rx.isEmpty()) {
            list << QRegularExpression("^" + rx + "$", QRegularExpression::CaseInsensitiveOption);
        }
    }
    return list;
}

FileScannerWorker::FileScannerWorker(QObject *parent) : QObject(parent) {}

void FileScannerWorker::setRootDir(const QString &dir) { rootDir = dir; }

void FileScannerWorker::start() {
    QElapsedTimer timer;
    timer.start();
    scanDir(rootDir);
    emit finished(timer.elapsed());
}

void FileScannerWorker::scanDir(const QString &rootPath) {
    auto chunk = QStringList();
    auto const chunkSize = 200;
    auto queue = QQueue<QString>();
    queue.enqueue(rootPath);

    while (!queue.isEmpty()) {
        auto dirPath = queue.dequeue();
        auto dir = QDir(dirPath);
        auto entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        for (auto const &fi : entries) {
            if (fi.isDir()) {
                queue.enqueue(normalizeFilePath(fi.absoluteFilePath()));
            } else {
                auto rel = QDir(rootDir).relativeFilePath(fi.absoluteFilePath());
                chunk << normalizeFilePath(rel);
                if (chunk.size() >= chunkSize) {
                    emit filesChunkFound(chunk);
                    chunk.clear();
                    QThread::msleep(20);
                }
            }
        }
    }
    if (!chunk.isEmpty()) {
        emit filesChunkFound(chunk);
    }
}

FilesList::FilesList(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    list = new QListWidget(this);
    excludeEdit = new QLineEdit(this);
    showEdit = new QLineEdit(this);
    loadingWidget = new LoadingWidget(this);

    list->setAlternatingRowColors(true);

    showEdit->setClearButtonEnabled(true);
    showEdit->setPlaceholderText(tr("Files to show (e.g. main;*.cpp;*.h)"));
    showEdit->setToolTip(showEdit->placeholderText());

    excludeEdit->setClearButtonEnabled(true);
    excludeEdit->setPlaceholderText(tr("Files to hide (e.g. build;*.o*;.git)"));
    excludeEdit->setToolTip(excludeEdit->placeholderText());

    layout->addWidget(loadingWidget);
    layout->addWidget(list);
    layout->addWidget(showEdit);
    layout->addWidget(excludeEdit);

    connect(list, &QListWidget::itemClicked, this, [this](auto *it) {
        auto fileName = this->directory + QDir::separator() + it->text();
        fileName = QDir::toNativeSeparators(fileName);
        emit fileSelected(fileName);
    });
    connect(excludeEdit, &QLineEdit::textChanged, this, &FilesList::scheduleUpdateList);
    connect(showEdit, &QLineEdit::textChanged, this, &FilesList::scheduleUpdateList);

    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(300);
    connect(updateTimer, &QTimer::timeout, this, [this]() { updateList(filesList, true); });
}

void FilesList::setExcludeListEnabled(bool state) { this->excludeEdit->setEnabled(state); }

void FilesList::setExcludeList(const QString &filter) {
    this->excludeEdit->setText(filter);
    scheduleUpdateList();
}

void FilesList::setShowListEnabled(bool state) { this->showEdit->setEnabled(state); }

void FilesList::setShowList(const QString &filter) {
    this->showEdit->setText(filter);
    scheduleUpdateList();
}

void FilesList::setDir(const QString &dir) {
    clear();
    directory = normalizeDirPath(dir);

    auto *thread = new QThread;
    auto *worker = new FileScannerWorker;
    worker->moveToThread(thread);
    worker->setRootDir(dir);
    connect(worker, &FileScannerWorker::filesChunkFound, this, [=, this](const QStringList &chunk) {
        QMetaObject::invokeMethod(
            this,
            [=, this]() {
                filesList.append(chunk);
                loadingWidget->setToolTip(QString(tr("Total %1 files")).arg(filesList.size()));
                updateList(chunk, false);
            },
            Qt::QueuedConnection);
    });
    connect(worker, &FileScannerWorker::finished, this, [=, this](qint64 ms) {
        qDebug() << "Scan finished in" << ms << "ms";
        worker->deleteLater();
        thread->quit();
        loadingWidget->stop();
    });
    connect(thread, &QThread::started, worker, &FileScannerWorker::start);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
    loadingWidget->start();
    loadingWidget->setToolTip({});
}

void FilesList::setFiles(const QStringList &files) {
    filesList = files;
    scheduleUpdateList();
}

void FilesList::clear() {
    filesList.clear();
    list->clear();
    directory.clear();
}

QStringList FilesList::currentFilteredFiles() const {
    auto res = QStringList();
    for (auto i = 0; i < list->count(); ++i) {
        res << list->item(i)->text();
    }
    return res;
}

void FilesList::scheduleUpdateList() {
    if (updateTimer->isActive()) {
        updateTimer->stop();
    }
    updateTimer->start();
}

void FilesList::updateList(const QStringList &files, bool clearList) {
    auto excludes = toRegexList(excludeEdit->text().split(';', Qt::SkipEmptyParts));
    auto shows = toRegexList(showEdit->text().split(';', Qt::SkipEmptyParts));
    auto showTokens = showEdit->text().toLower().split(';', Qt::SkipEmptyParts);
    auto filtered = QStringList();
    auto count = 0;

    if (clearList) {
        list->clear();
    }

    for (auto const &rel : files) {
        auto normPath = normalizeFilePath(rel);
        auto segments = normPath.split('/', Qt::SkipEmptyParts);
        auto excluded = false;

        for (auto const &rx : excludes) {
            for (auto const &segment : segments) {
                if (rx.match(segment).hasMatch()) {
                    excluded = true;
                    break;
                }
            }
            if (excluded) {
                break;
            }
        }
        if (excluded) {
            continue;
        }

        if (!shows.isEmpty() || !showTokens.isEmpty()) {
            auto matched = false;
            for (auto const &rx : shows) {
                for (auto const &segment : segments) {
                    if (rx.match(segment).hasMatch()) {
                        matched = true;
                        break;
                    }
                }
                if (matched) {
                    break;
                }
            }

            if (!matched && !showTokens.isEmpty()) {
                for (auto const &token : showTokens) {
                    for (auto const &segment : segments) {
                        if (segment.toLower().contains(token)) {
                            matched = true;
                            break;
                        }
                    }
                    if (matched) {
                        break;
                    }
                }
            }

            if (!matched) {
                continue;
            }
        }

        filtered << rel;
        if (++count % 200 == 0) {
            QCoreApplication::processEvents();
        }
    }

    filtered.sort(Qt::CaseInsensitive);
    for (auto const &rel : filtered) {
        auto *item = new QListWidgetItem(QDir::toNativeSeparators(rel));
        item->setToolTip(QDir::toNativeSeparators(directory + rel));
        list->addItem(item);
    }
    if (clearList) {
        emit filtersChanged();
    }
}
