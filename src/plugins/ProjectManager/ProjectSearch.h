#ifndef PROJECTSEARCH_H
#define PROJECTSEARCH_H

#include <QWidget>

namespace Ui {
class ProjectSearchGUI;
}

class DirectoryModel;
class QTreeWidgetItem;

class ProjectSearch : public QWidget {
    Q_OBJECT

  public:
    explicit ProjectSearch(QWidget *parent, DirectoryModel *m);
    ~ProjectSearch();

  public slots:
    void on_searchButton_clicked();
  private slots:
    void on_line_found(QTreeWidgetItem *item);

  private:
    Ui::ProjectSearchGUI *ui;
    DirectoryModel *model;
};

#endif // PROJECTSEARCH_H
