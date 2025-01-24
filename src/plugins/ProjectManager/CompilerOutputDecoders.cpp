#include "CompilerOutputDecoders.h"

#include <QDir>
#include <QFileInfo>

void GccOutputDetector::processLine(const QString &line, const QString &) {
    auto match = regionPattern.match(line);
    if (match.hasMatch()) {
        if (!currentStatus.message.isEmpty()) {
            m_compileStatuses.append(currentStatus);
        }

        // gcc errors/warnings start at  line 1, we start and line 0
        auto fileName = match.captured(1);
        auto lineNmber = match.captured(2).toInt() - 1;
        auto columnNumber = match.captured(3).toInt() - 1;
        auto type = match.captured(4);
        auto message = match.captured(5);

        auto fi = QFileInfo(fileName);
        auto displayName = fi.fileName();
        if (!displayName.isEmpty()) {
            currentStatus =
                CompileStatus{fileName, displayName, lineNmber, columnNumber, type, message};
        }
    } else if (!line.isEmpty()) {
        if (!currentStatus.fileName.isEmpty()) {
            currentStatus.message += "\n";
            currentStatus.message += line;
        }
    } else {
        // Empty line (can be treated as a boundary or EOF)
        if (!currentStatus.message.isEmpty()) {
            m_compileStatuses.append(currentStatus);
            currentStatus.message.clear();
            currentStatus.fileName.clear();
        }
    }
}

QList<CompileStatus> GccOutputDetector::foundStatus() {
    QList<CompileStatus> result = m_compileStatuses;
    m_compileStatuses.clear(); // Reset the internal list
    return result;
}

void GccOutputDetector::endOfOutput() {
    if (!currentStatus.message.isEmpty()) {
        m_compileStatuses.append(currentStatus);

        // Clear the current message buffer
        currentStatus.message.clear();
        currentStatus.fileName.clear();
    }
}

GeneralDetector::~GeneralDetector() { qDeleteAll(detectors); }

void GeneralDetector::processLine(const QString &line, const QString &sourceDir) {
    for (auto detector : detectors) {
        detector->processLine(line, sourceDir);
    }
}

QList<CompileStatus> GeneralDetector::foundStatus() {
    auto list = QList<CompileStatus>();
    for (auto detector : detectors) {
        list += detector->foundStatus();
    }
    return list;
}

void GeneralDetector::endOfOutput() {
    for (auto detector : detectors) {
        detector->endOfOutput();
    }
}

void GeneralDetector::add(OutputDetector *detector) { detectors.append(detector); }

void GeneralDetector::remove(OutputDetector *detector) {
    if (detectors.removeOne(detector)) {
        delete detector;
    }
}

void ClOutputDetector::processLine(const QString &line, const QString &) {
    static QRegularExpression clPattern(
        R"(([a-zA-Z]:\\[^:]+|\S+)\((\d+),(\d+)\):\s+(\w+)\s+(\w+):\s+(.+))");

    auto match = clPattern.match(line);
    if (match.hasMatch()) {
        auto fileName = match.captured(1);
        auto lineNumber = match.captured(2).toInt();
        auto columnNumber = match.captured(3).toInt();
        auto type = match.captured(4);
        // auto code = match.captured(5);
        auto message = match.captured(6);

        auto fi = QFileInfo(fileName);
        auto displayName = fi.fileName();
        compileStatuses.append(
            CompileStatus{fileName, displayName, lineNumber, columnNumber, type, message});
    }
}

QList<CompileStatus> ClOutputDetector::foundStatus() {
    QList<CompileStatus> result = compileStatuses;
    compileStatuses.clear();
    return result;
}

void ClOutputDetector::endOfOutput() {
    // nothing
}

void CargoOutputDetector::processLine(const QString &line, const QString &sourceDir) {
    static QRegularExpression errorPattern(R"(^error: (.+))");
    static QRegularExpression locationPattern(R"(^\s*-->\s*([^:]+):(\d+):(\d+))");

    auto errorMatch = errorPattern.match(line);
    if (errorMatch.hasMatch()) {
        if (!currentStatus.message.isEmpty()) {
            m_compileStatuses.append(currentStatus);
        }
        currentStatus = CompileStatus{"", "", -1, -1, "error", errorMatch.captured(1)};
    } else {
        auto locationMatch = locationPattern.match(line);
        if (locationMatch.hasMatch()) {
            currentStatus.fileName = sourceDir + QDir::separator() + locationMatch.captured(1);
            currentStatus.displayName = QFileInfo(currentStatus.fileName).fileName();
            currentStatus.row = locationMatch.captured(2).toInt() - 1;
            currentStatus.col = locationMatch.captured(3).toInt() - 1;
        } else if (!line.trimmed().isEmpty()) {
            if (!currentStatus.message.isEmpty()) {
                currentStatus.message += "\n";
            }
            currentStatus.message += line.trimmed();
        } else if (!currentStatus.message.isEmpty() && !currentStatus.fileName.isEmpty()) {
            m_compileStatuses.append(currentStatus);
            currentStatus = {};
        }
    }
}

QList<CompileStatus> CargoOutputDetector::foundStatus() {
    QList<CompileStatus> result = m_compileStatuses;
    m_compileStatuses.clear();
    return result;
}

void CargoOutputDetector::endOfOutput() {
    if (!currentStatus.message.isEmpty()) {
        m_compileStatuses.append(currentStatus);
        currentStatus = {};
    }
    accumulatedMessage.clear();
}
