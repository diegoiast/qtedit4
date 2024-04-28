/**
 * \brief Fifth demo - mixing the text editor with syntyax highliting
 *
 * This example shows how to mix the syntax highlighter with the new
 * text editor control.
 *
 * The app creates a main window with all function on the toolbar, without
 * any menus.
 *
 * When building from QtCreator, open the Projects tab, in the Build &Run
 * section choose this demo, and in Working direcotry type %{sourceDir}.
 */

#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QTimer>
#include <QMainWindow>
#include <QFileDialog>
#include <QSyntaxHighlighter>
#include <QToolButton>
#include <QPushButton>
#include <QDebug>
#include <QCommonStyle>
#include <QMessageBox>

#include "qsvsh/qsvcolordef.h"
#include "qsvsh/qsvcolordeffactory.h"
#include "qsvsh/qsvlangdef.h"
#include "qsvsh/qsvlangdeffactory.h"
#include "qsvsh/qsvsyntaxhighlighter.h"

#include "qate/qateblockdata.h"
#include "qsvte/qsvtextedit.h"
#include "qsvte/qsvtextoperationswidget.h"
#include "qsvte/qsvsyntaxhighlighterbase.h"

class MyHighlighter: public QsvSyntaxHighlighter, public QsvSyntaxHighlighterBase {
public:
	MyHighlighter(QTextDocument *parent): QsvSyntaxHighlighter(parent) {
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
	QsvSyntaxHighlighter::highlightBlock(text);
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
	Qate::BlockData    *blockData = nullptr;
	
	if (userData == nullptr){
		blockData =  new Qate::BlockData();
		block.setUserData(blockData);
	} else {
		blockData = dynamic_cast<Qate::BlockData*>(userData);
	}
	return blockData;
}


class MainWindow : public QMainWindow
{
	Q_OBJECT
	QsvTextEdit *editor;
	QsvColorDefFactory 	 * defColors;
	QsvLangDef 		 * langDefinition;
	MyHighlighter    	 * highlight;
	QsvTextOperationsWidget  * textOpetations;
public:
	MainWindow( const QString &file )
	{
		QString dataPath  = QDir::currentPath();
		//QsvLangDefFactory::getInstanse()->addMimeTypes( "data/mime.types" );
		QsvLangDefFactory::getInstanse()->loadDirectory( "data/langs/" );
		editor           = new QsvTextEdit(this, nullptr);
		defColors        = new QsvColorDefFactory( "data/colors/kate.xml" );
		langDefinition   = QsvLangDefFactory::getInstanse()->getHighlight("1.cpp");
		highlight        = new MyHighlighter(editor->document());
        textOpetations   = new QsvTextOperationsWidget(editor);
		
		if (!defColors->isValid() || !langDefinition->isValid()) {
			QMessageBox::information(this, 
				tr("Read documentation"),
				tr("Cannot find color or language definition.\n\n"
				"Are you running the app from the top dir?\n"
				"If running from QtCreator, set up working directory to %{sourceDir}\n"
				"See documentation in main1.cpp"
			));
		}
		
		highlight->setColorsDef(defColors);
		highlight->setHighlight(langDefinition);
		editor->setHighlighter(highlight);
		editor->setFrameStyle(QFrame::NoFrame);
		
		editor->findChild<QWidget*>("banner")
			->findChild<QToolButton*>("closeButton")
			->setIcon( style()->standardIcon(QStyle::SP_DockWidgetCloseButton) );
		
		QPushButton *p;
		textOpetations->initSearchWidget();
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
		
		textOpetations->initReplaceWidget();
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
		
		setCentralWidget(editor);
		showMaximized();
		editor->displayBannerMessage(tr("Click \"Open\" if you dare"));
		
		if (!file.isEmpty())
			loadFile(file);
		else {
			setWindowTitle("QtSourceView demo5 - qedit");
			QFile f("demos/demo5/readme.txt");
			if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
				return;
			editor->setPlainText(f.readAll());
			editor->removeModifications();
		}

#if 0
		// tests for defaults
		editor->setMarkCurrentLine(false);
		editor->setShowLineNumbers(true);
		editor->setShowMargins(true);
		editor->setTabSize(8);
		editor->setTabIndents(true);
		editor->setInsertSpacesInsteadOfTabs(true);
		editor->setShowWhiteSpace(true);
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
//		if (editor->isModified()){
//			editor->save();
//		}
		
		editor->clear();
		langDefinition = QsvLangDefFactory::getInstanse()->getHighlight(filename);
		highlight->setHighlight(langDefinition);
		editor->loadFile(filename);
		editor->removeModifications();
		setWindowTitle(filename);
	}

private:
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w( a.arguments().count()>=2 ? a.arguments().at(1) : QString() );
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	return a.exec();
}

#include "main5.moc"
