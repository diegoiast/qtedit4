#include "CommitModel.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSize>
#include <qabstractitemmodel.h>

namespace {
void addHorizontalConnectors(QString &graphString) {
    if (graphString.contains('/')) {
        return;
    }
    auto starPos = graphString.lastIndexOf('*');
    if (starPos != -1) {
        auto branchPoint = graphString.left(starPos).lastIndexOf(QRegularExpression("[|\\\\/+]"));
        if (branchPoint != -1) {
            for (auto i = branchPoint + 1; i < starPos; ++i) {
                if (graphString[i] == ' ') {
                    graphString[i] = '-';
                }
            }
        }
    }
}

} // namespace

CommitModel::CommitModel(QObject *parent) : QAbstractListModel(parent) {}

int CommitModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_commits.count();
}

QVariant CommitModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_commits.count()) {
        return {};
    }

    auto &commit = m_commits[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return QString("%1 %2").arg(commit.hash.left(7), commit.message);
    case Qt::SizeHintRole:
        return QSize(-1, m_rowHeight); // Fixed row height
    case GraphRole:
        return commit.graphLines;
    case HashRole:
        return commit.hash;
    case ParentsRole:
        return QVariant(commit.parents);
    case MessageRole:
        return commit.message;
    case AuthorRole:
        return commit.author;
    case DateRole:
        return QVariant(commit.date);
    }
    return {};
}

QString CommitModel::detectRepoRoot(const QString &filePath) const {
    QProcess p;
    p.setWorkingDirectory(QFileInfo(filePath).absolutePath());
    p.start(gitBinary, {"rev-parse", "--show-toplevel"});
    p.waitForFinished();
    return QString::fromUtf8(p.readAllStandardOutput()).trimmed();
}

bool CommitModel::loadFileHistory(const QString &file) {
    beginResetModel();
    m_commits.clear();

    QString repoRoot = detectRepoRoot(file);
    if (repoRoot.isEmpty()) {
        endResetModel();
        return false;
    }

    QProcess p;
    p.setWorkingDirectory(repoRoot);
    auto args = QStringList{"log", "--graph", "--pretty=format:%x01%H%x02%P%x02%an%x02%ai%x02%s"};
    if (!file.isEmpty()) {
        args << "--" << file;
    }
    p.start(gitBinary, args);
    p.waitForFinished();

    auto output = QString::fromUtf8(p.readAllStandardOutput());
    auto lines = output.split('\n', Qt::SkipEmptyParts);
    auto graphLinesBuffer = QStringList{};

    for (auto &line : lines) {
        auto sepPos = line.indexOf(QChar(0x01));
        if (sepPos == -1) {
            // Graph-only line
            graphLinesBuffer.append(line);
        } else {
            // Commit line
            CommitInfo commit;
            commit.graphLines = graphLinesBuffer;
            commit.graphLines.append(line.left(sepPos));
            graphLinesBuffer.clear();

            auto commitData = line.mid(sepPos + 1);
            auto parts = commitData.split(QChar(0x02));
            if (parts.count() < 5) {
                continue;
            }

            auto dateString = parts[3];
            dateString.replace(10, 1, 'T');

            commit.hash = parts[0];
            commit.parents = parts[1].split(' ', Qt::SkipEmptyParts);
            commit.author = parts[2];
            commit.date = QDateTime::fromString(dateString, Qt::ISODate);
            commit.message = parts[4];
            for (auto &graphLine : commit.graphLines) {
                addHorizontalConnectors(graphLine);
            }
            m_commits.append(commit);
        }
    }
    endResetModel();
    return true;
}

bool CommitModel::loadProjectHistory(const QString &filePath) { return loadFileHistory({}); }
