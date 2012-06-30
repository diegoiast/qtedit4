#include <QMainWindow>
#include <QUrl>
#include <QMessageBox>
#include <QAction>
#include <QStringList>

#include <qmdiserver.h>

#include "texteditor_plg.h"
#include "src/widgets/qmdieditor.h"
#include "qsvdefaulthighlighter.h"


TextEditorPlugin::TextEditorPlugin()
{
	name = tr("Text editor plugin - based on QtSourceView");
	author = tr("Diego Iastrubni <elcuco@kde.org>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = false;
	
	actionNewFile	= new QAction( tr("New blank file"), this  );
	actionNewCPP	= new QAction( tr("New source"), this );
	actionNewHeader	= new QAction( tr("New header"), this );
	myNewActions	= new QActionGroup(this);
	myNewActions->addAction( actionNewFile );
	myNewActions->addAction( actionNewCPP );
	myNewActions->addAction( actionNewHeader );

	hl_manager = Qate::HighlightDefinitionManager::instance();
	mimes = new Qate::MimeDatabase();
	hl_manager->setMimeDatabase(mimes);
	hl_manager->registerMimeTypes();

	connect( myNewActions, SIGNAL(triggered(QAction*)), this, SLOT(fileNew(QAction*)));
}

TextEditorPlugin::~TextEditorPlugin()
{
}

void	TextEditorPlugin::showAbout()
{
	QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "This pluging gives a QtSourceView based text edito" );
}

QWidget*	TextEditorPlugin::getConfigDialog()
{
	return NULL;
}

Qate::MimeType TextEditorPlugin::getMimeByExt(const QString &fileName) const
{
	QFileInfo fi(fileName);
	QString extension;

	if (fi.suffix().isEmpty())
		extension = fi.fileName();
	else
		extension = QString("*.%1").arg(fi.suffix());

	foreach(Qate::MimeType mime, mimes->mimeTypes() ) {
		foreach(Qate::MimeGlobPattern pattern, mime.globPatterns() ) {
			if (extension == pattern.regExp().pattern())
				return mime;
		}
	}
	return Qate::MimeType();
}


QString		TextEditorPlugin::findDefinitionId(const Qate::MimeType &mimeType, bool considerParents) const
{
    QString definitionId = hl_manager->definitionIdByAnyMimeType(mimeType.aliases());
    if (definitionId.isEmpty() && considerParents) {
	definitionId = hl_manager->definitionIdByAnyMimeType(mimeType.subClassesOf());
	if (definitionId.isEmpty()) {
	    foreach (const QString &parent, mimeType.subClassesOf()) {
		const Qate::MimeType &parentMimeType =  mimes->findByType(parent);
		definitionId = findDefinitionId(parentMimeType, considerParents);
	    }
	}
    }
    return definitionId;
}


QActionGroup*	TextEditorPlugin::newFileActions()
{
	return myNewActions;
}

QStringList	TextEditorPlugin::myExtensions()
{
	QStringList s;
	s << tr("Sources"	, "EditorPlugin::myExtensions")	+ " (*.c *.cpp *.cxx *.h *.hpp *.hxx *.inc)";
	s << tr("Headers"	, "EditorPlugin::myExtensions")	+ " (*.h *.hpp *.hxx *.inc)";
	s << tr("Text files"	, "EditorPlugin::myExtensions")	+ " (*.txt)";
		s << tr("Qt project"	, "EditorPlugin::myExtensions")	+ " (*.pro *.pri)";
	s << tr("All files"	, "EditorPlugin::myExtensions")	+ " (*.*)";
	
	return s;
}

int	TextEditorPlugin::canOpenFile( const QString fileName )
{
	QUrl u(fileName);

	// if the scheme is a single line, lets assume this is a windows drive
	if (u.scheme().length() != 1)
		if ( (u.scheme().toLower() != "file") && (!u.scheme().isEmpty()) )
			return -2;

	if (fileName.endsWith(".c", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".cpp", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".cxx", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".h", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".hpp", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".hxx", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".inc", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".pro", Qt::CaseInsensitive))
		return 5;
	else if (fileName.endsWith(".pri", Qt::CaseInsensitive))
		return 5;
	else return 1;
}

#include "qate/highlighter.h"
#include "qate/highlightdefinition.h"
#include "qate/defaultcolors.h"
#include "qate/context.h"
#include "qatehighlighter.h"

bool	TextEditorPlugin::openFile( const QString fileName, int x, int y, int z )
{
	qmdiEditor *editor = new qmdiEditor( fileName, dynamic_cast<QMainWindow*>(mdiServer) );

#if 0
	DefaultHighlighter *highlighter = new DefaultHighlighter(editor);
	editor->setHighlighter(highlighter);
	highlighter->rehighlight();
#else
	Qate::MimeType m;
	QSharedPointer<TextEditor::Internal::HighlightDefinition>  highlight_definition;
	QateHighlighter *highlighter = new QateHighlighter;
	Qate::DefaultColors::ApplyToHighlighter(highlighter);
	QString definitionId;

	mimes->findByFile(fileName);

	if (m.isNull())
		m = getMimeByExt(fileName);

	definitionId = hl_manager->definitionIdByMimeType(m.type());
	if (definitionId.isEmpty())
		definitionId = findDefinitionId(m,true);

	if (!definitionId.isEmpty()) {
		qDebug("Using %s for %s" , qPrintable(definitionId), qPrintable(fileName));
		highlight_definition = hl_manager->definition(hl_manager->definitionIdByMimeType(m.type()));
		if (!highlight_definition.isNull()) {
			highlighter->setDefaultContext(highlight_definition->initialContext());
		}
	} else {
		qDebug("No highlighter found for %s" , qPrintable(fileName));
	}

	editor->setHighlighter(highlighter);
	highlighter->rehighlight();
	editor->removeModifications();
#endif
	mdiServer->addClient(editor);

	// TODO
	// 1) move the cursor as specified in the parameters
	// 2) return false if the was was not open for some reason
	return true;
	Q_UNUSED( x );
	Q_UNUSED( y );
	Q_UNUSED( z );
}

void	TextEditorPlugin::getData()
{
}

void	TextEditorPlugin::setData()
{
}

void TextEditorPlugin::fileNew( QAction * )
{
	qmdiEditor *editor = new qmdiEditor( tr("NO NAME"), dynamic_cast<QMainWindow*>(mdiServer) );
	DefaultHighlighter *highlighter = new DefaultHighlighter(editor);
	editor->setHighlighter(highlighter);
	mdiServer->addClient( editor );
}
