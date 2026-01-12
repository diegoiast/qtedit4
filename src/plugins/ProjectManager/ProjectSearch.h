#pragma once

#include <QWidget>

namespace Ui {
class ProjectSearchGUI;
}

class ProjectBuildModel;
class QTreeWidgetItem;

struct FoundData {
    std::string line;
    size_t lineNumber = 0;
};

class ProjectSearch : public QWidget {
    Q_OBJECT

  public:
    explicit ProjectSearch(QWidget *parent, ProjectBuildModel *m);
    ~ProjectSearch();
    auto setFocusOnSearch() -> void;

    auto getSearchPath() const -> const QString;
    auto setSearchPath(const QString &) -> void;
    auto getSearchPattern() const -> const QString;
    auto setSearchPattern(const QString &) -> void;
    auto getSearchInclude() const -> const QString;
    auto setSearchInclude(const QString &) -> void;
    auto getSearchExclude() const -> const QString;
    auto setSearchExclude(const QString &) -> void;
    auto getCollapseFiles() const -> bool;
    auto setCollapseFiles(bool status) -> void;

  public slots:
    auto updateProjectList() -> void;
    auto searchButton_clicked() -> void;
    void file_searched(QString fullFileName, QString shortFileName, QList<FoundData> *foundData);

  private:
    Ui::ProjectSearchGUI *ui;
    ProjectBuildModel *model;
};
