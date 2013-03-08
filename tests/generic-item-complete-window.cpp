#include <QTreeView>
#include <QVBoxLayout>
#include <QTimer>
#include <QLineEdit>
#include <QListView>
#include <QTableView>
#include <QHeaderView>
#include <vector>

#include "src/plugins/ProjectManager/GenericItems.h"
#include "generic-item-complete-window.h"
#include <QMouseEvent>


SuggestionModel::SuggestionModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	//m_suggestions = suggestions;
}

int SuggestionModel::rowCount(const QModelIndex &parent) const
{
	return m_suggestions.count();
}

int SuggestionModel::columnCount(const QModelIndex &parent) const
{
	return 3;
}

QVariant SuggestionModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();


	switch (index.column()) {
	case 0:	return m_suggestions.at(index.row()).value;
	case 1:	return m_suggestions.at(index.row()).item->getDisplay(0);
	case 2:	return m_suggestions.at(index.row()).item->fullPath;
	default:
		return QVariant();
		break;
	}
}

void SuggestionModel::setSuggestions(QList<Suggestion> newSuggestions)
{
	beginResetModel();
	m_suggestions = newSuggestions;
	endResetModel();
}


GenericItemWindow::GenericItemWindow() : QMainWindow(NULL,0)
{
	m_tv = NULL;
	m_edit = NULL;
	m_suggestionsList = NULL;
	m_model = NULL;
	m_suggestionModel = NULL;
	QTimer::singleShot(0,this,SLOT(initGUI()));
}

void GenericItemWindow::initGUI()
{
	QWidget *w = new QWidget;
	QVBoxLayout *l = new QVBoxLayout;
	m_model = new FoldersModel;
	m_suggestionModel = new SuggestionModel;
	m_suggestionsList = new QTableView;
	m_edit = new QLineEdit;
	m_tv = new QTreeView;
	m_tv->setModel(m_model);
	m_suggestionsList->setModel(m_suggestionModel);
	m_suggestionsList->setShowGrid(false);
	m_suggestionsList->horizontalHeader()->hide();
	m_suggestionsList->verticalHeader()->hide();
	m_suggestionsList->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::MinimumExpanding);
	m_suggestionsList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_edit->setFocus();
	QTimer::singleShot(0,m_edit,SLOT(setFocus()));

	layout()->setContentsMargins(0,0,0,0);
	layout()->setMargin(0);
	l->addWidget(m_tv);
	l->addWidget(m_edit);
	l->addWidget(m_suggestionsList);
	l->setContentsMargins(0,0,0,0);
	l->setMargin(0);
	w->setLayout(l);
	setCentralWidget(w);
	QTimer::singleShot(0,this,SLOT(initFolders()));
	m_edit->installEventFilter(this);
	connect(m_edit,SIGNAL(textEdited(QString)),this,SLOT(fillSuggestions(QString)));
}

bool GenericItemWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() != QEvent::KeyPress) {
		return QObject::eventFilter(obj, event);
	}
	QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
	QModelIndex i = m_suggestionsList->currentIndex();
	int step = 0;
	int current = i.row();
	int delta = m_suggestions.count() - current;

	switch (keyEvent->key()) {
	case Qt::Key_Down:
	case Qt::Key_PageDown:
		delta = m_suggestions.count() - current;
		if (keyEvent->key() == Qt::Key_Down)
			step = 1;
		else
			step =  (delta / 10) + 1;
		if (current + step >= m_suggestions.count())
			step = m_suggestions.count() - current - 1;
		if (step < 0)
			step = 0;
		break;
	case Qt::Key_Up:
	case Qt::Key_PageUp:
		delta = current;
		if (keyEvent->key() == Qt::Key_Up)
			step = -1;
		else
			step =  -(delta / 10) - 1;
		if (current + step < 0)
			step = -1;
		if (current + step < 0)
			step = 0;
		break;
	}

	if (step != 0 ) {
		qDebug("step is %d (delta=%d): %d/%d", step, delta, i.row(), m_suggestions.count() );

		i = m_suggestionModel->index(i.row()+step, 0);
		m_suggestionsList->setCurrentIndex(i);
		return true;
	}

	return QObject::eventFilter(obj, event);
}

void GenericItemWindow::initFolders()
{
  m_model->processDir("/home/elcuco/src/googlecode/qtedit4/");
//	m_model->processDir("/home/elcuco/src/qtedit4/trunk");
}

int LevenshteinDistance(QString s, QString t);
int DiegoDistance(QString s, QString t);

bool suggestionLessThan(const Suggestion& s1, const Suggestion& s2)
{
    return s1.value < s2.value;
}

bool suggestionBiggerThan(const Suggestion& s1, const Suggestion& s2)
{
    return s1.value > s2.value;
}

void GenericItemWindow::fillSuggestions(QString s)
{
	m_suggestions.clear();
	fillSuggestions(s,static_cast<const FileItem *>(m_model->getGenericRootItem()),m_suggestions);
	qSort(m_suggestions.begin(), m_suggestions.end(),suggestionBiggerThan);
	m_suggestionModel->setSuggestions(m_suggestions);
	m_suggestionsList->resizeColumnsToContents();
	m_suggestionsList->setCurrentIndex(m_suggestionModel->index(0, 0));
}

void GenericItemWindow::fillSuggestions(QString requestedSuggestion, const FileItem *item, QList<Suggestion>&suggestions)
{
	if (requestedSuggestion.isEmpty())
		return;

	foreach(GenericItem *child,item->subChildren){
		const FileItem *i = static_cast<const FileItem*>(child);

		if (!i->isDirectory) {
			QString itemName = i->getDisplay(0);
			int d = 100;
//			d = DiegoDistance(s1,s2);
			if (d>90) {
				d = LevenshteinDistance(requestedSuggestion,itemName);
				if (d>10) {
					continue;
				}
			}
			Suggestion ss;
			ss.item  = i;
			ss.value = 100 - d;

			if (itemName.startsWith(requestedSuggestion))
				ss.value *= 2;
			else if (itemName.contains(requestedSuggestion))
				ss.value *= 1.5;

			if (d<10 && (itemName.endsWith(".c") || itemName.endsWith(".cpp") || itemName.endsWith(".cxx")) )
				ss.value = ss.value * 1.2 + 20;
			if (d<10 && (itemName.endsWith(".h") || itemName.endsWith(".hpp")))
				ss.value = ss.value * 1.2 - 10;
			if (itemName.endsWith(".o") || itemName.endsWith(".so") ||
				 itemName.endsWith(".obj") || itemName.endsWith(".dll")  ||
				 itemName.endsWith(".lib") || itemName.endsWith(".a") ||
				 itemName.endsWith(".exe") || itemName.endsWith("a.out")
				 )
				ss.value *= 0.7;

			if (!itemName.contains('.'))
				ss.value *= 0.7;

			if (ss.value >= 90)
				suggestions<<ss;
		}
		else
			fillSuggestions(requestedSuggestion,i,suggestions);
	}
}

// http://stackoverflow.com/questions/3437404/min-and-max-in-c
// see also: http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

int minimum(int a, int b, int c )
{
	if (a<b)
		return min(c,b);
	else
		return min(c,a);
}

int DiegoDistance(QString s, QString t)
{
	int d = abs(s.length()-t.length());
	if (t.startsWith(s,Qt::CaseInsensitive))
		return d;
//	if (d>5)
		return 100;
//	return d;
}

// http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.23
int LevenshteinDistanceIterative(QString s, QString t)
{
	const size_t len1 = s.size(), len2 = t.size();
	std::vector<unsigned int> col(len2+1), prevCol(len2+1);

	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++) {
		col[0] = i+1;
		for (unsigned int j = 0; j < len2; j++)
			col[j+1] = min( min( 1 + col[j], 1 + prevCol[1 + j]),
								prevCol[j] + (s[i]==t[j] ? 0 : 1) );
		col.swap(prevCol);
	}
	return prevCol[len2];
}

int LevenshteinDistanceRecursive(QString s, QString t)

{
	int len_s = s.length(), len_t = t.length(), cost = 0;

	if(s[0] != t[0])
		cost = 1;

	if (len_s == 0)
		return len_t;
	else if (len_t == 0)
		return len_s;

	QString s1 = s; // s[1..len_s-1];
	s1.chop(1);
	s1.remove(0,1);
	QString t1 = t; //t[1..len_t-1];
	t1.chop(1);
	t1.remove(0,1);
	return minimum(
		LevenshteinDistance(s1, t ) + 1,
		LevenshteinDistance( s, t1) + 1,
		LevenshteinDistance(s1, t1) + cost
	);
}

int LevenshteinDistance(QString s, QString t)
{
	s = s.toLower();
	t = t.toLower();
#if 0
	return LevenshteinDistanceRecursive(s,t);
#else
	return LevenshteinDistanceIterative(s,t);
#endif
}
