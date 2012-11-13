#include <QMainWindow>
#include <QAbstractTableModel>

class QTreeView;
class QLineEdit;
class FoldersModel;
class QListView;
class FileItem;
class QTableView;

struct Suggestion {
	const FileItem* item;
	int value;
};

class SuggestionModel : public QAbstractTableModel {
public:
	SuggestionModel(QObject *parent=NULL);
	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	void setSuggestions(QList<Suggestion> newSuggestions);
private:
	QList<Suggestion> m_suggestions;
};


class GenericItemWindow: public QMainWindow {
	Q_OBJECT
public:
	QTreeView *m_tv;
	QLineEdit *m_edit;
	QTableView *m_suggestionsList;
	FoldersModel *m_model;
	QList<Suggestion> m_suggestions;
	SuggestionModel *m_suggestionModel;

	GenericItemWindow();
	void fillSuggestions(QString s1, const FileItem *item, QList<Suggestion> &m_suggestions);
public slots:
	void initGUI();
	void initFolders();

	void fillSuggestions(QString);
};
