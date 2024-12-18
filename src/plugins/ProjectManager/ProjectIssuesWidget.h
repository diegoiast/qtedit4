#pragma once

#include <QAbstractItemModel>
#include <QSet>
#include <QVector>
#include <QWidget>

namespace Ui {
class ProjectIssuesWidget;
}

struct CompileStatus {
    QString fileName;
    int row;
    int col;
    QString type;
    QString message;
};

class CompileStatusModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    explicit CompileStatusModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void clearAll();
    void addItem(const CompileStatus &status);

    void setWarningsVisible(bool visible);
    void setErrorsVisible(bool visible);
    void setOthersVisible(bool visible);

    bool areWarningsVisible() const;
    bool areErrorsVisible() const;
    bool areOthersVisible() const;

  private:
    QVector<CompileStatus> m_statuses;
    QVector<CompileStatus> m_filteredStatuses;
    QStringList m_headers;
    bool m_showWarnings;
    bool m_showErrors;
    bool m_showOthers;

    void applyFilter();
    bool shouldShowStatus(const CompileStatus &status) const;
};

class ProjectIssuesWidget : public QWidget {
    Q_OBJECT

  public:
    explicit ProjectIssuesWidget(QWidget *parent = nullptr);
    ~ProjectIssuesWidget();

    void processLine(const QString &line);

  protected:
    void changeEvent(QEvent *e);

  private:
    CompileStatusModel *model;
    Ui::ProjectIssuesWidget *ui;
};
