#pragma once

#include <QList>
#include <QRegularExpression>
#include <QString>

struct CompileStatus {
    QString fileName;
    QString displayName;
    int row;
    int col;
    QString type;
    QString message;
};

class OutputDetector {
  public:
    virtual ~OutputDetector() = default;

    virtual void processLine(const QString &line) = 0;

    // Returns the collected compile status and resets the list
    virtual QList<CompileStatus> foundStatus() = 0;

    virtual void endOfOutput() = 0;
};

class GeneralDetector : public OutputDetector {
  public:
    GeneralDetector() = default;
    ~GeneralDetector() override;
    virtual void processLine(const QString &line) override;
    virtual QList<CompileStatus> foundStatus() override;
    virtual void endOfOutput() override;

    void add(OutputDetector *detector);
    void remove(OutputDetector *detector);

  private:
    QList<OutputDetector *> detectors;
};

class GccOutputDetector : public OutputDetector {
  public:
    GccOutputDetector() = default;
    ~GccOutputDetector() override = default;

    void processLine(const QString &line) override;
    QList<CompileStatus> foundStatus() override;
    virtual void endOfOutput() override;

  private:
    // GCC output pattern for errors and warnings: file:line:column: message
    QRegularExpression regionPattern =
        QRegularExpression(R"(([a-zA-Z]:\\[^:]+|\S+):(\d+):(\d+):\s+(.+):\s+(.+))");

    QList<CompileStatus> m_compileStatuses;
    CompileStatus currentStatus = {};
};

class ClOutputDetector : public OutputDetector {
  public:
    void processLine(const QString &line);
    QList<CompileStatus> foundStatus();
    void endOfOutput();

  public:
    QList<CompileStatus> compileStatuses;
};
