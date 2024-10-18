#pragma once

#include <QWidget>

namespace Ui {
class ProjectSearchGUI;
}

class DirectoryModel;
class QTreeWidgetItem;

struct FoundData {
    std::string line;
    size_t lineNumber = 0;
};

class ProjectSearch : public QWidget {
    Q_OBJECT

  public:
    explicit ProjectSearch(QWidget *parent, DirectoryModel *m);
    ~ProjectSearch();
    void setFocusOnSearch();

    auto getSearchPattern() -> const QString;
    auto setSearchPattern(const QString) -> void;
    auto getSearchInclude() -> const QString;
    auto setSearchInclude(const QString) -> void;
    auto getSearchExclude() -> const QString;
    auto setSearchExclude(const QString) -> void;

  private slots:
    void searchButton_clicked();
    void file_searched(QString fullFileName, QString shortFileName, QList<FoundData> *foundData);

  private:
    Ui::ProjectSearchGUI *ui;
    DirectoryModel *model;
};
