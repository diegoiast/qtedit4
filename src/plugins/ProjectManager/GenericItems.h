#pragma once

#include <QAbstractItemModel>
#include <QDir>
#include <QList>
#include <QRunnable>
#include <QSortFilterProxyModel>
#include <QString>

bool FilenameMatches(const QString &fileName, const QString &goodList, const QString &badList);

class FilesWorker : public QObject, public QRunnable {
    Q_OBJECT

    QString _rootPath;

  public:
    explicit FilesWorker(const QString &rootPath, QObject *parent = nullptr);
    virtual void run() override;

  signals:
    void started(const QString &rootPath);
    void filesLoaded(const QStringList &files);
    void finished();
};

class DirectoryModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    DirectoryModel(QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    const QString &getItem(size_t i) const { return fileList[i]; }
    QString displayForItem(size_t i) const;
    QString fileNameForItem(size_t i) const;

    void removeAllDirs();
    void addDirectory(const QString &path);
    void removeDirectory(const QString &path);

    QStringList fileList;
    QStringList directoryList;

  public slots:
    void scanStarted(const QString &rootPath);
    void newFiles(const QStringList &files);
    void scanFinished();

  signals:
    void didFinishLoading();

  private:
    void removeDirectoryImpl(const QDir &directory);
};

class FilterOutProxyModel : public QSortFilterProxyModel {
  public:
    explicit FilterOutProxyModel(QObject *parent = nullptr);
    void setFilterWildcards(const QString &wildcards);
    void setFilterOutWildcard(const QString &wildcard);

  protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    static int countOccurrences(const QString &str, const QString &targets);

  private:
    QString m_filterWildcards;
    QString m_filterOutWildcard;
};
