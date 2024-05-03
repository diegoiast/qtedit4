#include <QAbstractTableModel>
#include <QDir>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QStringList>

#include <src/plugins/ProjectManager/GenericItems.h>

class QTreeView;
class QLineEdit;
class FoldersModel;
class QListView;
class FileItem;
class QTableView;

struct Suggestion {
    QString fileName;
    int value;
};

class SuggestionModel : public QAbstractTableModel {
  public:
    SuggestionModel(QObject *parent = NULL);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    void setSuggestions(QList<Suggestion> newSuggestions);
    const Suggestion &getItem(size_t i) const { return m_suggestions[i]; }

  private:
    QList<Suggestion> m_suggestions;
};

class GenericItemWindow : public QMainWindow {
    Q_OBJECT
  public:
    QLineEdit *m_files_filter;
    QListView *filesList;
    QLineEdit *filesFilterEx;
    QTableView *m_suggestionsList;

    QSortFilterProxyModel *filesFilterModel;
    DirectoryModel *directoryModel;
    QList<Suggestion> m_suggestions;
    SuggestionModel *m_suggestionModel;

    GenericItemWindow();
    void fillSuggestions(QString s1, QList<Suggestion> &m_suggestions);

  protected:
    bool eventFilter(QObject *obj, QEvent *event);
  public slots:
    void initGUI();
    void initFolders();

    void fillSuggestions(QString);
};
