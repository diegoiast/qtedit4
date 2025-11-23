#include "CommitModel.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSize>
#include <qabstractitemmodel.h>
#include <qnamespace.h>

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

bool CommitModel::loadFileHistory(const QString &file, bool scopeLogToFile) {
    beginResetModel();
    m_commits.clear();

    repoRoot = detectRepoRoot(file);
    if (repoRoot.isEmpty()) {
        endResetModel();
        return false;
    }

    QProcess p;
    p.setWorkingDirectory(repoRoot);
    auto args = QStringList{"log", "--graph", "--pretty=format:%x01%H%x02%P%x02%an%x02%ai%x02%s"};
    if (scopeLogToFile && !file.isEmpty()) {
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

bool CommitModel::loadProjectHistory(const QString &filePath) {
    return loadFileHistory(filePath, false);
}

CommitInfo CommitModel::getCommitInfo(const QString &sha1) const {
    for (const auto &commit : m_commits) {
        if (commit.hash == sha1) {
            return commit;
        }
    }
    return {};
}

FullCommitInfo CommitModel::getFullCommitInfo(const QString &sha1) const {
    auto s = getRawCommitDiff(sha1);
    return parseFullCommitInfo(s);
}

QString CommitModel::getRawCommitDiff(const QString &sha1) const {
    if (repoRoot.isEmpty()) {
        return {};
    }
    QProcess p;
    p.setWorkingDirectory(repoRoot);
    p.start(gitBinary, {"show", sha1});
    p.waitForFinished();
    return QString::fromUtf8(p.readAllStandardOutput());
}

FullCommitInfo CommitModel::parseFullCommitInfo(const QString &rawInfo) const {
    FullCommitInfo result;
    result.raw = rawInfo;
    if (result.raw.isEmpty()) {
        return result;
    }

    QStringView content(result.raw);

    // 1. Headers
    int headerEndPos = content.indexOf(u"\n\n");
    if (headerEndPos == -1) {
        result.body = content;
        return result;
    }

    QStringView headers = content.left(headerEndPos);
    content = content.mid(headerEndPos + 2);

    for (const auto &line : headers.split('\n')) {
        if (line.startsWith(u"commit ")) {
            result.hash = line.mid(7);
        } else if (line.startsWith(u"Author: ")) {
            result.author = line.mid(8);
        } else if (line.startsWith(u"Date:   ")) {
            result.date = line.mid(8).trimmed();
        }
    }

    // 2. Diff section
    int diffStartPos = content.indexOf(u"\ndiff --git");
    QStringView messageBlock;
    if (diffStartPos == -1) {
        messageBlock = content;
        result.diff = QStringView();
    } else {
        messageBlock = content.left(diffStartPos);
        result.diff = content.mid(diffStartPos + 1);
    }

    // 3. Subject and Body from message block (un-indenting by 4 spaces)
    int firstLineStart = messageBlock.indexOf(QRegularExpression(R"(\S)"));
    if (firstLineStart == -1) { // Empty message
        return result;
    }

    // Find body (text after first empty line)
    int bodyPos = messageBlock.indexOf(u"\n\n", firstLineStart);
    if (bodyPos == -1) {
        result.subject = messageBlock.mid(firstLineStart).trimmed();
    } else {
        result.subject = messageBlock.mid(firstLineStart, bodyPos - firstLineStart).trimmed();
        result.body = messageBlock.mid(bodyPos + 2).trimmed();
    }

    return result;
}
