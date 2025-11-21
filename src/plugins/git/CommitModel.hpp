#pragma once

#include <QAbstractListModel>
#include <QVariant>
#include <QVector>

Q_DECLARE_METATYPE(QVector<int>)

struct CommitEntry {
    QString sha;
    QStringList parents;
    QString subject;
    QString author;
    QString date;
    QStringList refs;

    int column;
    QVector<int> parentColumns;
};

class CommitModel : public QAbstractListModel {
    Q_OBJECT
  public:
    CommitModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loadFileHistory(const QString &filePath);
    bool loadProjectHistory(const QString &filePath);
    inline QString getGitBinary() const { return gitBinary; }
    inline void setGitBinary(const QString &s) { gitBinary = s; }

  protected:
    void buildTree(QList<QByteArray> &lines);
    void assignLanes();
    int findCommitIndexBySha1(const QString &sha) const;

  private:
    QString gitBinary = "git";
    QString detectRepoRoot(const QString &filePath) const;
    QString getGitLogForFile(const QString &repoPath, const QString &filePath) const;
    QVector<CommitEntry> parseGitLog(const QString &raw) const;
    void assignGraphColumns(QVector<CommitEntry> &commits);

    QVector<CommitEntry> m_commits;
};
