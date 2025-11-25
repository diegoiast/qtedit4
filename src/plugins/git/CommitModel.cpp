#include "CommitModel.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLatin1String>
#include <QList>
#include <QProcess>
#include <QRegularExpression>
#include <QSize>
#include <qabstractitemmodel.h>
#include <qnamespace.h>

#include <memory>

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
    if (rawInfo.isEmpty()) {
        return result;
    }

    result.raw = std::make_shared<const QString>(rawInfo);
    QStringView content(*result.raw);

    // 1. Headers
    int headerEndPos = content.indexOf(QLatin1String("\n\n"));
    if (headerEndPos == -1) {
        // No headers found, something is wrong, but we can treat it all as body.
        // Body must be an owned QString.
        result.body = content.toString();
        return result;
    }

    QStringView headers = content.left(headerEndPos);
    for (const auto &line : headers.split('\n')) {
        if (line.startsWith(QLatin1String("commit "))) {
            result.hash = line.mid(7);
        } else if (line.startsWith(QLatin1String("Author: "))) {
            result.author = line.mid(8);
        } else if (line.startsWith(QLatin1String("Date:   "))) {
            result.date = line.mid(8).trimmed();
        }
    }

    // 2. Message and Diff blocks
    QStringView bodyAndDiff = content.mid(headerEndPos + 2);
    int diffStartPos = bodyAndDiff.indexOf(QLatin1String("diff --git"));
    QStringView messageBlock;
    if (diffStartPos == -1) {
        messageBlock = bodyAndDiff;
    } else {
        messageBlock = bodyAndDiff.left(diffStartPos);
    }

    // 3. Subject and Body from message block
    int firstLineStart = messageBlock.indexOf(QRegularExpression(R"(\S)"));
    if (firstLineStart != -1) {
        const QRegularExpression separator(QLatin1String("\\n\\s*\\n"));
        const auto match = separator.matchView(messageBlock, firstLineStart);

        if (!match.hasMatch()) {
            result.subject = messageBlock.mid(firstLineStart).trimmed();
        } else {
            const int bodyPos = match.capturedStart();
            const int separatorLength = match.capturedLength();
            result.subject = messageBlock.mid(firstLineStart, bodyPos - firstLineStart).trimmed();

            QStringView rawBody = messageBlock.mid(bodyPos + separatorLength);
            QList<QStringView> bodyLines = rawBody.split('\n', Qt::KeepEmptyParts);

            QStringList processedBodyLines;
            processedBodyLines.reserve(bodyLines.size());
            for (QStringView line : bodyLines) {
                if (line.startsWith(QLatin1String("    "))) {
                    processedBodyLines.append(line.sliced(4).toString());
                } else {
                    processedBodyLines.append(line.toString());
                }
            }
            result.body = processedBodyLines.join('\n').trimmed();
        }
    }

    // 4. Parse file diffs
    if (diffStartPos != -1) {
        QStringView diffsBlock = bodyAndDiff.mid(diffStartPos);
        int currentPos = 0;
        while (currentPos < diffsBlock.size()) {
            int nextPos = diffsBlock.indexOf(QLatin1String("\ndiff --git"), currentPos + 1);
            if (nextPos != -1) {
                nextPos++; // include the \n
            }
            int endOfBlock = (nextPos == -1) ? diffsBlock.size() : nextPos;

            QStringView currentDiff = diffsBlock.mid(currentPos, endOfBlock - currentPos);
            if (currentDiff.isEmpty()) {
                break;
            }

            FileDiff fileDiff;
            fileDiff.diff_content = currentDiff;

            int firstLineEnd = currentDiff.indexOf('\n');
            QStringView firstLine = currentDiff.left(firstLineEnd);
            int b_part_start = firstLine.lastIndexOf(QLatin1String(" b/"));
            if (b_part_start != -1) {
                fileDiff.filename = firstLine.mid(b_part_start + 3);
            }

            result.files.append(fileDiff);

            if (nextPos == -1) {
                break;
            }
            currentPos = nextPos;
        }
    }

    return result;
}
