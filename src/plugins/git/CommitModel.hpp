#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QVariant>
#include <QVector>

struct CommitInfo {
    QString hash;
    QStringList parents;
    QString message;
    QString author;
    QDateTime date;
    QStringList graphLines;
};

class CommitModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum Roles {
        HashRole = Qt::UserRole + 1,
        ParentsRole,
        MessageRole,
        AuthorRole,
        DateRole,
        GraphRole
    };

    CommitModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool loadFileHistory(const QString &filePath);
    bool loadProjectHistory(const QString &filePath);

    QString detectRepoRoot(const QString &filePath) const;

  protected:
  private:
    QString gitBinary = "git";
    QVector<CommitInfo> m_commits;
    int m_rowHeight = 22;
};
