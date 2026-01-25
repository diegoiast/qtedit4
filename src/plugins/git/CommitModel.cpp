#include "CommitModel.hpp"

#include <QDebug>
#include <QLatin1String>
#include <QList>
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

FullCommitInfo FullCommitInfo::parse(const QString &rawInfo) {
    FullCommitInfo result;
    if (rawInfo.isEmpty()) {
        return result;
    }

    result.raw = std::make_shared<const QString>(rawInfo);
    QStringView content(*result.raw);

    auto headerEndPos = content.indexOf(QLatin1String("\n\n"));
    if (headerEndPos == -1) {
        result.body = content.toString();
        return result;
    }

    auto headers = content.left(headerEndPos);
    for (auto const &line : headers.split('\n')) {
        if (line.startsWith(QLatin1String("commit "))) {
            result.hash = line.mid(7);
        } else if (line.startsWith(QLatin1String("Author: "))) {
            result.author = line.mid(8);
        } else if (line.startsWith(QLatin1String("Date:   "))) {
            result.date = line.mid(8).trimmed();
        }
    }

    auto bodyAndDiff = content.mid(headerEndPos + 2);
    auto diffStartPos = bodyAndDiff.indexOf(QLatin1String("diff --git"));
    auto messageBlock = QStringView();
    if (diffStartPos == -1) {
        messageBlock = bodyAndDiff;
    } else {
        messageBlock = bodyAndDiff.left(diffStartPos);
    }

    auto firstLineStart = messageBlock.indexOf(QRegularExpression(R"(\S)"));
    if (firstLineStart != -1) {
        auto static separator = QRegularExpression(QLatin1String("\\n\\s*\\n"));
        auto const match = separator.matchView(messageBlock, firstLineStart);

        if (!match.hasMatch()) {
            result.subject = messageBlock.mid(firstLineStart).trimmed();
        } else {
            auto bodyPos = match.capturedStart();
            auto separatorLength = match.capturedLength();
            auto rawBody = messageBlock.mid(bodyPos + separatorLength);
            auto bodyLines = rawBody.split('\n', Qt::KeepEmptyParts);
            auto processedBodyLines = QStringList();

            result.subject = messageBlock.mid(firstLineStart, bodyPos - firstLineStart).trimmed();
            processedBodyLines.reserve(bodyLines.size());
            for (auto line : bodyLines) {
                if (line.startsWith(QLatin1String("    "))) {
                    processedBodyLines.append(line.sliced(4).toString());
                } else {
                    processedBodyLines.append(line.toString());
                }
            }
            result.body = processedBodyLines.join('\n').trimmed();
        }
    }
    if (diffStartPos != -1) {
        auto diffsBlock = bodyAndDiff.mid(diffStartPos);
        auto currentPos = 0;
        while (currentPos < diffsBlock.size()) {
            auto nextPos = diffsBlock.indexOf(QLatin1String("\ndiff --git"), currentPos + 1);
            if (nextPos != -1) {
                // include the \n
                nextPos++;
            }
            auto endOfBlock = (nextPos == -1) ? diffsBlock.size() : nextPos;
            auto currentDiff = diffsBlock.mid(currentPos, endOfBlock - currentPos);
            if (currentDiff.isEmpty()) {
                break;
            }

            FileDiff fileDiff;
            fileDiff.diff_content = currentDiff;
            auto firstLineEnd = currentDiff.indexOf('\n');
            auto firstLine = currentDiff.left(firstLineEnd);
            auto b_part_start = firstLine.lastIndexOf(QLatin1String(" b/"));
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

void CommitModel::setContent(const QString &output) {
    beginResetModel();
    m_commits.clear();

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
}

CommitInfo CommitModel::getCommitInfo(const QString &sha1) const {
    for (const auto &commit : m_commits) {
        if (commit.hash == sha1) {
            return commit;
        }
    }
    return {};
}
