#ifndef PROJECTSEARCH_H
#define PROJECTSEARCH_H

#include <QWidget>

namespace Ui {
class ProjectSearchGUI;
}

class ProjectSearch : public QWidget {
    Q_OBJECT

  public:
    explicit ProjectSearch(QWidget *parent = nullptr);
    ~ProjectSearch();

  private:
    Ui::ProjectSearchGUI *ui;
};

#endif // PROJECTSEARCH_H
