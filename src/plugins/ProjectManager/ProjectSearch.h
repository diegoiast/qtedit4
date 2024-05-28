#ifndef PROJECTSEARCH_H
#define PROJECTSEARCH_H

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

  public slots:
    void on_searchButton_clicked();
  private slots:
    void file_searched(QString fullFileName, QString shortFileName, QList<FoundData> *data);

  private:
    Ui::ProjectSearchGUI *ui;
    DirectoryModel *model;
};

#endif // PROJECTSEARCH_H
