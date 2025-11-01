/**
 * \file FileList.hpp
 * \brief Definition of a flat file lister
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#pragma once

#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QWidget>

class QLineEdit;
class QListWidget;
class FileScannerWorker;
class FileFilterWorker;
class LoadingWidget;

class FileScannerWorker : public QObject {
    Q_OBJECT
  public:
    explicit FileScannerWorker(QObject *parent = nullptr);
    void setRootDir(const QString &dir);
    const QString &getRootDir() { return rootDir; }

  public slots:
    void start();
    void requestStop() { shouldStop = true; }
    bool requestedStop() const { return shouldStop; }

  signals:
    void filesChunkFound(const QStringList &chunk);
    void finished(qint64 elapsedMs);

  private:
    void scanDir(const QString &rootPath);
    QString rootDir;
    bool shouldStop = false;
};

class FilesList : public QWidget {
    Q_OBJECT
  public:
    explicit FilesList(QWidget *parent = nullptr);

    void setShowList(const QString &filter);
    void setShowListEnabled(bool state);
    void setExcludeList(const QString &filter);
    void setExcludeListEnabled(bool state);

    void setFiles(const QStringList &files);
    void setDir(const QString &dir);
    const QString &getDir() const { return directory; };
    void clear();
    QStringList currentFilteredFiles() const;
    const QStringList &getAllFiles() const { return allFilesList; }

  signals:
    void fileSelected(const QString &filename);
    void filtersChanged();

  private slots:
    void scheduleUpdateList();
    void updateList(const QStringList &chunk, bool clearList);

  private:
    bool matchesFilters(const QString &filename, const QList<QRegularExpression> &excludeRegexes,
                        const QList<QRegularExpression> &showRegexes,
                        const QStringList &showTokens) const;

    LoadingWidget *loadingWidget = nullptr;
    QListWidget *displayList = nullptr;
    QLineEdit *excludeEdit = nullptr;
    QLineEdit *showEdit = nullptr;

    QString directory;
    QStringList allFilesList;

    FileScannerWorker *worker = nullptr;
    QThread *scanThread = nullptr;
    FileFilterWorker *filterWorker = nullptr;
    QTimer *updateTimer = nullptr;
};
