#include "CommitModel.hpp"
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <qnamespace.h>
#include <qtypes.h>

CommitModel::CommitModel(QObject *parent) : QAbstractListModel(parent) {
    qRegisterMetaType<QVector<int>>("QVector<int>");
}

int CommitModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_commits.size();
}

QVariant CommitModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    auto &c = m_commits.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return c.subject;
    case Qt::ToolTipRole:
        return c.sha + " " + c.author + " " + c.date;
    case Qt::UserRole + 1:
        return c.column;
    case Qt::UserRole + 2:
        return QVariant::fromValue(c.parentColumns);
    case Qt::UserRole + 3:
        return c.sha;
    case Qt::UserRole + 4:
        return c.author;
    case Qt::UserRole + 5:
        return c.date;
    case Qt::UserRole + 6:
        return c.refs;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> CommitModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "subject";
    roles[Qt::UserRole + 1] = "column";
    roles[Qt::UserRole + 2] = "parentColumns";
    roles[Qt::UserRole + 3] = "sha";
    roles[Qt::UserRole + 4] = "author";
    roles[Qt::UserRole + 5] = "date";
    roles[Qt::UserRole + 6] = "refs";
    return roles;
}

QString CommitModel::detectRepoRoot(const QString &filePath) const {
    QProcess p;
    p.setWorkingDirectory(QFileInfo(filePath).absolutePath());
    p.start(gitBinary, {"rev-parse", "--show-toplevel"});
    p.waitForFinished();
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}

QString CommitModel::getGitLogForFile(const QString &repoPath, const QString &filePath) const {
    QProcess p;
    p.setWorkingDirectory(repoPath);
    p.start(gitBinary, {"log", "--pretty=format:%H|%P|%s|%an|%ad|%D", "--date=short",
                        "--date-order", "--", filePath});
    p.waitForFinished();
    return QString::fromUtf8(p.readAllStandardOutput());
}

QVector<CommitEntry> CommitModel::parseGitLog(const QString &raw) const {
    QStringList lines = raw.split('\n', Qt::SkipEmptyParts);
    QVector<CommitEntry> out;
    out.reserve(lines.size());

    for (auto const &line : lines) {
        auto parts = line.split('|');
        if (parts.size() < 6) {
            continue;
        }

        CommitEntry c;
        c.sha = parts[0];
        c.parents = parts[1].split(' ', Qt::SkipEmptyParts);
        c.subject = parts[2];
        c.author = parts[3];
        c.date = parts[4];
        c.refs = parts[5].split(',', Qt::SkipEmptyParts);
        c.column = -1;
        out.push_back(c);
    }
    return out;
}

void CommitModel::assignGraphColumns(QVector<CommitEntry> &commits) {
    QHash<QString, int> laneOf;
    auto nextLane = 0;

    for (auto c : commits) {
        if (!laneOf.contains(c.sha)) {
            laneOf[c.sha] = nextLane++;
        }

        c.column = laneOf[c.sha];
        c.parentColumns.clear();
        c.parentColumns.reserve(c.parents.size());

        auto first = true;
        for (auto const &p : c.parents) {
            if (first) {
                if (!laneOf.contains(p)) {
                    laneOf[p] = c.column;
                }
                c.parentColumns.push_back(laneOf[p]);
                first = false;
            } else {
                if (!laneOf.contains(p)) {
                    laneOf[p] = nextLane++;
                }
                c.parentColumns.push_back(laneOf[p]);
            }
        }
    }
}

bool CommitModel::loadFileHistory(const QString &file) {
    beginResetModel();
    m_commits.clear();

    QProcess p;
    QStringList args = {
        "log",
        "--first-parent",
        "--pretty=format:%H%x1f%P%x1f%an%x1f%ad%x1f%s%x1f%D",
        "--date=short",
        "--",
        file,
    };

    p.setWorkingDirectory(detectRepoRoot(file));
    p.start(gitBinary, args);
    if (!p.waitForFinished()) {
        return false;
    }
    auto lines = p.readAllStandardOutput().split('\n');
    buildTree(lines);
    endResetModel();
    return !m_commits.isEmpty();
}

bool CommitModel::loadProjectHistory(const QString &filePath) {
    beginResetModel();
    m_commits.clear();

    QProcess p;
    QStringList args = {
        "log",
        "--first-parent",
        "--pretty=format:%H%x1f%P%x1f%an%x1f%ad%x1f%s%x1f%D",
        "--date=short",
    };

    p.setWorkingDirectory(detectRepoRoot(filePath));
    p.start(gitBinary, args);
    if (!p.waitForFinished()) {
        return false;
    }
    auto lines = p.readAllStandardOutput().split('\n');
    buildTree(lines);
    endResetModel();
    return !m_commits.isEmpty();
}

void CommitModel::buildTree(QList<QByteArray> &lines) {
    for (const QByteArray &ln : lines) {
        if (ln.isEmpty()) {
            continue;
        }

        QList<QByteArray> parts = ln.split('\x1f');
        if (parts.size() < 6) {
            continue;
        }

        CommitEntry c;
        c.sha = parts[0];
        c.parents = QString(parts[1]).split(' ', Qt::SkipEmptyParts);
        c.author = parts[2];
        c.date = parts[3];
        c.subject = parts[4];

        QString refText = parts[5];
        if (!refText.isEmpty()) {
            c.refs = refText.split(", ");
        }
        c.column = 0;
        c.parentColumns.clear();
        m_commits.append(c);
    }
    assignLanes();
}

void CommitModel::assignLanes() {
    QVector<int> active; // sha → column mapping

    for (auto i = 0; i < m_commits.size(); ++i) {
        auto &c = m_commits[i];
        auto col = active.indexOf(i);
        if (col < 0) {
            col = active.size();
            active.append(i);
        }

        c.column = col;
        c.parentColumns.clear();
        for (auto const &pSha : c.parents) {
            int parentIndex = findCommitIndexBySha1(pSha);
            if (parentIndex < 0) {
                continue;
            }
            auto pCol = active.indexOf(parentIndex);
            if (pCol < 0) {
                pCol = active.size();
                active.append(parentIndex);
            }
            c.parentColumns.append(pCol);
        }
        active.removeAll(i);
    }
}

int CommitModel::findCommitIndexBySha1(const QString &sha) const {
    for (qsizetype i = 0, n = m_commits.size(); i < n; ++i) {
        if (m_commits.at(i).sha == sha) {
            return i;
        }
    }
    return -1;
}
