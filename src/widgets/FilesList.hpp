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

  public slots:
    void start();

  signals:

    void filesChunkFound(const QStringList &chunk);
    void finished(qint64 elapsedMs);

  private:
    void scanDir(const QString &rootPath);
    QString rootDir;
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
    QString getDir() const { return directory; };
    void clear();
    QStringList currentFilteredFiles() const;
    const QStringList &getAllFiles() const { return filesList; }

  signals:
    void fileSelected(const QString &filename);
    void filtersChanged();

  private slots:
    void scheduleUpdateList();
    void updateList(const QStringList &files, bool clearList);

  private:
    LoadingWidget *loadingWidget = nullptr;
    QListWidget *list = nullptr;
    QLineEdit *excludeEdit = nullptr;
    QLineEdit *showEdit = nullptr;

    QString directory;
    QStringList filesList;

    QThread *filterThread = nullptr;
    FileFilterWorker *filterWorker = nullptr;
    QTimer *updateTimer = nullptr;
};
