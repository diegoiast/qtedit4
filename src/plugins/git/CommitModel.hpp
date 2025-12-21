#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QVariant>
#include <QVector>

#include <memory>

struct CommitInfo {
    QString hash;
    QStringList parents;
    QString message;
    QString author;
    QDateTime date;
    QStringList graphLines;
};

struct FileDiff {
    QStringView filename;
    QStringView diff_content;
};

struct FullCommitInfo {
    std::shared_ptr<const QString> raw;
    QStringView hash;
    QStringView author;
    QStringView date;
    QStringView subject;
    QString body;
    QList<FileDiff> files;
    static FullCommitInfo parse(const QString &rawInfo);
};

class CommitModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum Roles {
        HashRole = Qt::UserRole + 1,
        ParentsRole,
        MessageRole,
        BodyRole,
        AuthorRole,
        DateRole,
        GraphRole
    };

    CommitModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setContent(const QString &gitLogOutput);
    CommitInfo getCommitInfo(const QString &sha1) const;

  protected:
  private:
    QVector<CommitInfo> m_commits;
    int m_rowHeight = 22;
};
