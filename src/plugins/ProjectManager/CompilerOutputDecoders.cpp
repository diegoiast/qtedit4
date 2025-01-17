#include "CompilerOutputDecoders.h"

void GccOutputDetector::processLine(const QString &line) {
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
        currentStatus = CompileStatus{fileName, lineNmber, columnNumber, type, message};
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
    m_compileStatuses.clear();  // Reset the internal list
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
