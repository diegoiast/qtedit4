#pragma once

#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QStringList>

class SharedHistoryModel : public QObject {
    Q_OBJECT

  public:
    SharedHistoryModel(QObject *parent = nullptr) : QObject(parent) {}

    void addToHistory(const QString &entry);
    void setHistory(QStringList &strings);
    inline const QStringList &getHistory() const { return history; }

    inline int getMaxHistorySize() const { return maxHistorySize; }
    void setMaxHistorySize(int newSize);

  signals:
    void historyUpdated(const QStringList &history);

  private:
    QStringList history;
    int maxHistorySize = 100;
};

class HistoryLineEdit : public QLineEdit {
    Q_OBJECT

  public:
    HistoryLineEdit(QWidget *parent = nullptr);
    HistoryLineEdit(SharedHistoryModel *sharedModel = nullptr, QWidget *parent = nullptr);
    void setHistoryModel(SharedHistoryModel *m);

  protected:
    void keyPressEvent(QKeyEvent *event) override;

  private slots:
    void addToHistory();
    void updateCompleter(const QStringList &history);

  private:
    void navigateHistory(int direction);

    SharedHistoryModel *historyModel = nullptr;
    QCompleter *completer = nullptr;
    int historyIndex = -1;
};
