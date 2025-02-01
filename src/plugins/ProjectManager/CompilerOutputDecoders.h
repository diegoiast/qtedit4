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
    QString detectedBy;
};

class OutputDetector {
  public:
    virtual ~OutputDetector() = default;

    virtual bool processLine(const QString &line, const QString &sourceDir) = 0;

    // Returns the collected compile status and resets the list
    virtual QList<CompileStatus> foundStatus() = 0;

    virtual void endOfOutput() = 0;
};

class GeneralDetector : public OutputDetector {
  public:
    GeneralDetector() = default;
    ~GeneralDetector() override;
    virtual bool processLine(const QString &line, const QString &sourceDir) override;
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

    virtual bool processLine(const QString &lin, const QString &sourceDir) override;
    virtual QList<CompileStatus> foundStatus() override;
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
    virtual bool processLine(const QString &line, const QString &sourceDir);
    virtual QList<CompileStatus> foundStatus();
    virtual void endOfOutput();

  public:
    QList<CompileStatus> compileStatuses;
};

class CargoOutputDetector : public OutputDetector {
  public:
    CargoOutputDetector() = default;
    ~CargoOutputDetector() override = default;

    virtual bool processLine(const QString &line, const QString &sourceDir) override;
    virtual QList<CompileStatus> foundStatus() override;
    virtual void endOfOutput() override;

  private:
    // Rust/Cargo output pattern for errors and warnings
    QRegularExpression regionPattern = QRegularExpression(
        R"(^(?:error|warning)(?:\[([^\]]+)\])?: (.+)\n\s*-->\s*([^:]+):(\d+):(\d+))");

    QList<CompileStatus> m_compileStatuses;
    CompileStatus currentStatus = {};
    QString accumulatedMessage;
};

class GoLangOutputDetector : public OutputDetector {
  public:
    GoLangOutputDetector() = default;
    ~GoLangOutputDetector() override = default;
    virtual bool processLine(const QString &line, const QString &sourceDir) override;
    virtual void endOfOutput() override;
    virtual QList<CompileStatus> foundStatus() override;

  private:
    QList<CompileStatus> m_compileStatuses;
    CompileStatus currentStatus;
    QString accumulatedMessage;
};
