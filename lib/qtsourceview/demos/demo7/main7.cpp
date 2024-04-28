#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QTimer>
#include <QMainWindow>
#include <QWindow>
#include <QFileDialog>
#include <QSyntaxHighlighter>
#include <QToolButton>
#include <QPushButton>
#include <QDebug>
#include <QCommonStyle>
#include <QTextDocument>
#include <QTextBlock>

#include "qate/context.h"
#include "qate/highlighter.h"
#include "qate/highlightdefinition.h"
#include "qate/highlightdefinitionmanager.h"
#include "qate/defaultcolors.h"
#include "qate/qateblockdata.h"
#include "qate/mimedatabase.h"

//#include "qate/qateblockdata.h"
#include "qsvte/qsvtextedit.h"
#include "qsvte/qsvtextoperationswidget.h"
#include "qsvte/qsvsyntaxhighlighterbase.h"


class MyHighlighter: public TextEditor::Internal::Highlighter, public QsvSyntaxHighlighterBase
{
public:
	MyHighlighter(QTextDocument *parent): TextEditor::Internal::Highlighter(parent)
	{
		setMatchBracketList("{}()[]''\"\"");
	}

	virtual void highlightBlock(const QString &text) override;

	virtual void toggleBookmark(QTextBlock &block) override;

	virtual void removeModification(QTextBlock &block) override;

	virtual void setBlockModified(QTextBlock &block, bool on) override;

	virtual bool isBlockModified(QTextBlock &block) override;

	virtual bool isBlockBookmarked(QTextBlock &block) override;

	virtual Qate::BlockData::LineFlags getBlockFlags(QTextBlock &block) override;

	virtual void clearMatchData(QTextBlock &block) override;

	virtual void addMatchData(QTextBlock &block, Qate::MatchData m) override;

	virtual QList<Qate::MatchData> getMatches(QTextBlock &block) override;

	virtual QTextBlock getCurrentBlockProxy() override;

	Qate::BlockData *getBlockData(QTextBlock &block);
};

void MyHighlighter::highlightBlock(const QString &text) {
	QsvSyntaxHighlighterBase::highlightBlock(text);
	TextEditor::Internal::Highlighter::highlightBlock(text);
}

void MyHighlighter::toggleBookmark(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->toggleBookmark();
}

void MyHighlighter::removeModification(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->m_isModified = false;
}

void MyHighlighter::setBlockModified(QTextBlock &block, bool on) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->m_isModified =  on;
}

bool MyHighlighter::isBlockModified(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return false;
	return data->m_isModified;
}

bool MyHighlighter::isBlockBookmarked(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return 0;
	return data->isBookmark();
}

Qate::BlockData::LineFlags MyHighlighter::getBlockFlags(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return Qate::BlockData::LineFlag::Empty;
	return data->m_flags;
}

void MyHighlighter::clearMatchData(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
}

void MyHighlighter::addMatchData(QTextBlock &block, Qate::MatchData m) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return;
	data->matches << m;
}

QList<Qate::MatchData> MyHighlighter::getMatches(QTextBlock &block) {
	Qate::BlockData *data = getBlockData(block);
	if (data == nullptr)
		return QList<Qate::MatchData>();
	return data->matches;
}

QTextBlock MyHighlighter::getCurrentBlockProxy() {
	return currentBlock();
}

Qate::BlockData *MyHighlighter::getBlockData(QTextBlock &block) {
	QTextBlockUserData *userData  = block.userData();
	Highlighter::BlockData  *blockData = nullptr;

	if (userData == nullptr){
		blockData =  new Highlighter::BlockData;
		block.setUserData(blockData);
	} else {
		blockData = dynamic_cast<Highlighter::BlockData*>(userData);
	}
	return blockData;
}


void ApplyTheme(const Qate::Theme &theme, QsvTextEdit *editor, TextEditor::Internal::Highlighter *highlight) {
	theme.applyToHighlighter(highlight);
	theme.applyToEditor(editor);

	QColor c;
	c = theme.getEditorColor(Qate::EditorLineNumbers);
	if (c.isValid())
		editor->setPanelColor(c);
	c = theme.getEditorColor(Qate::EditorCurrentLine);
	if (c.isValid())
		editor->setCurrentLineBackground(c);
	c = theme.getEditorColor(Qate::EditorModifiedLines);
	if (c.isValid())
		editor->setModifiedColor(c);
}

////////////////////////////////////////////////////////////////////////////////
//
class MainWindow :public  QMainWindow
{
	Q_OBJECT
	QsvTextEdit *editor;
	QsvTextOperationsWidget  *textOpetations;

	Qate::Theme theme;
	Qate::MimeDatabase                                        *mimes;
	Qate::HighlightDefinitionManager                          *hl_manager;
	MyHighlighter                                             *highlight;
	QSharedPointer<TextEditor::Internal::HighlightDefinition>  highlight_definition;

public:
	MainWindow( const QString &file )
	{
		editor     = new QsvTextEdit(this, nullptr);
		highlight  = new MyHighlighter(editor->document());
		editor->setHighlighter(highlight);
		editor->setFrameStyle(QFrame::NoFrame);
		editor->displayBannerMessage(tr("Click \"Open\" if you dare"), 3);
		editor->setFont( QFont("Courier new",14) );
		theme.setupDefaultColors();
//		theme.load("data/colors/default.theme");
		theme.load("data/colors/solarized-light.theme");
//                theme.load("data/colors/solarized-dark.theme");
//                theme.load("data/colors/breeze-dark.theme");
		ApplyTheme(theme, editor, highlight);

		mimes      = new Qate::MimeDatabase();
		hl_manager = Qate::HighlightDefinitionManager::instance();
		hl_manager->setMimeDatabase(mimes);
		hl_manager->registerMimeTypes();
		connect(hl_manager,SIGNAL(mimeTypesRegistered()),this,SLOT(setDefaultHighlight()));

		textOpetations = new QsvTextOperationsWidget(editor);
		textOpetations->initSearchWidget();
		textOpetations->initReplaceWidget();
		setCentralWidget(editor);
		setup_GUI();
		showMaximized();

		if (!file.isEmpty()) {
			loadFile(file);
		} else {
			setWindowTitle("QtSourceView/qate demo6 - qedit");
			loadFile("tests/highlight.pas");
		}
#if 0
		e->setMarkCurrentLine(false);
		// tests for defaults
		e->setShowLineNumbers(true);
		e->setShowMargins(true);
		e->setTabSize(8);
		e->setTabIndents(true);
		e->setInsertSpacesInsteadOfTabs(true);
		e->setShowWhiteSpace(true);
#endif
	}
	
public slots:
	void loadFile( QString filename ="" )
	{
		const QString dir;
		if (filename.isEmpty()) {
			filename = QFileDialog::getOpenFileName(this,tr("Load file"),dir);
			if (filename.isEmpty())
				return;
		}
//		TODO
//		if (e->isModified){
//			e->save();
//		}
		
		editor->clear();
//		langDefinition = QsvLangDefFactory::getInstanse()->getHighlight(filename);
//		highlight->setHighlight(langDefinition);
		editor->loadFile(filename);
		editor->removeModifications();
		setWindowTitle(filename);

		setHighlightDefinition("pascal");
	}

	void setHighlightDefinition(QString name, bool notifyOnError = false)
	{
		highlight_definition = hl_manager->definition( hl_manager->definitionIdByName(name) );
		if (highlight_definition.isNull()) {
			if (notifyOnError) {
				editor->displayBannerMessage("No highlight definition is found.");
			}
			return;
		}
		highlight->setDefaultContext(highlight_definition->initialContext());
		highlight->rehighlight();
	}

	void setDefaultHighlight() {
		setHighlightDefinition("Pascal", true);
	}

	void setup_GUI()
	{
		QPushButton *p;
		editor->findChild<QWidget*>("banner")
		      ->findChild<QToolButton*>("closeButton")
		      ->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );

		textOpetations->m_search->findChild<QAbstractButton*>("closeButton")->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );
		p = textOpetations->m_search->findChild<QPushButton*>("previousButton");
		p->setIcon( style()->standardIcon(QStyle::SP_ArrowUp) );
		p->setText("");
		p->setFlat(true);
		p->setAutoRepeat(true);
		p = textOpetations->m_search->findChild<QPushButton*>("nextButton");
		p->setIcon( style()->standardIcon(QStyle::SP_ArrowDown) );
		p->setText("");
		p->setFlat(true);
		p->setAutoRepeat(true);

		textOpetations->m_replace->findChild<QAbstractButton*>("closeButton")->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );
		p = textOpetations->m_search->findChild<QPushButton*>("previousButton");

		QToolBar *b = addToolBar( "" );
		b->addAction( tr("&New"), editor, SLOT(newDocument()))              ->setShortcut(QKeySequence("Ctrl+N"));
		b->addAction( tr("&Open"), this, SLOT(loadFile()))                  ->setShortcut(QKeySequence("Ctrl+O"));
		b->addAction( tr("&Save"), editor, SLOT(saveFile()))                ->setShortcut(QKeySequence("Ctrl+S"));
		b->addAction( tr("&Find"), textOpetations, SLOT(showSearch()))      ->setShortcut(QKeySequence("Ctrl+F"));
		b->addAction( tr("&Replace"), textOpetations, SLOT(showReplace()))  ->setShortcut(QKeySequence("Ctrl+R"));
		b->addAction( tr("Find &next"), textOpetations, SLOT(searchNext())) ->setShortcut(QKeySequence("F3"));
		b->setMovable(false);
		b->addAction( tr("Find &prev"), textOpetations, SLOT(searchPrev())) ->setShortcut(QKeySequence("Shift+F3"));
		b->setMovable(false);
	}
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w( a.arguments().count()>=2 ? a.arguments().at(1) : QString() );
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}

#include "main7.moc"
