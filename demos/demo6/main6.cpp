/**
 * \brief Sixth demo - Kate definitions based syntax highlighter
 *
 * The Kate project has a large amount of syntax definitions. The code
 * has been ripped from QtCreator, and now can be used as a simple
 * Qt application.
 *
 * When building from QtCreator, open the Projects tab, in the Build &Run
 * section choose this demo, and in Working direcotry type %{sourceDir}.
 */

#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QFile>
#include <QIODevice>
#include <QDir>

#include <unistd.h>

#include "qate/context.h"
#include "qate/highlighter.h"
#include "qate/highlightdefinition.h"
#include "qate/highlightdefinitionmanager.h"
#include "qate/defaultcolors.h"

#ifdef __WIN32
#   include <windows.h>
#else
#endif

#define DATA_DIR "data/qate/"
#define LANGUAGE  DATA_DIR "c.xml"
#define TEST_FILE "demos/demo6/main6.cpp"
#define LANG_NAME "C"

/* some examples for different formats inside comments */
// fixme todo  ###
/// doxygen comment
/// \brief bla bla bala

void load_text(QString fe, QPlainTextEdit *te );
QSharedPointer<TextEditor::Internal::HighlightDefinition> get_highlighter_definition(QString definitionFileName);

int main( int argc, char* argv[] )
{
	QApplication app( argc, argv );
	QMainWindow    *main_window = new QMainWindow;
	QPlainTextEdit *text_editor;

	Qate::Theme theme;
	Qate::MimeDatabase                                        *mimes;
	Qate::HighlightDefinitionManager                          *hl_manager;
	TextEditor::Internal::Highlighter                         *highlighter;
	QSharedPointer<TextEditor::Internal::HighlightDefinition>  highlight_definition;
	
	// create the main widget
	text_editor = new QPlainTextEdit(main_window);
	highlighter = new TextEditor::Internal::Highlighter(text_editor->document());

	theme.setupDefaultColors();
	theme.applyToHighlighter(highlighter);
	text_editor->setFont( QFont("Courier new",10) );
	
	// create the highlighters manager
	hl_manager = Qate::HighlightDefinitionManager::instance();

	// here is where the magic starts:
	//
	// The first option is to ask the manager for a specific file:
	//
	//	highlight_definition = hl_manager->definition(LANGUAGE);
	//
	// This will load the forward declarations and includes, and will work perfectly.
	// The downside, is that you still need to know the exact path, and if your
	// definitions are spread in several dirs - this will be a pain.
	//
	// The recommended way is to ask for a definition by name, or by mimetype
	// by calling definitionIdByAnyMimeType(). This example uses the former.
	//
	// Notes: 
	//  - if the syntax you loaded contains several includes or forward
	//    and you do not see them, this means that 
	//    Qate::HighlightDefinitionManager::registerMimeTypes() failed
	//  - if your application needs a single definition, you can there is 
	//    not need to setup the mime database, nor register the mime types
	//    see the first example.
	
#if 0
	highlight_definition = hl_manager->definition(LANGUAGE);
#else
	mimes = new Qate::MimeDatabase();
	hl_manager->setMimeDatabase(mimes);
	hl_manager->registerMimeTypes();

        // ugly - but... let the highlight manager build the mime DB
        // in  real life, you should the the highlight when the signal
        // definitionsMetaDataReady() is emmited
        // this code just waits "up to" a second for the HL to be available
	int timeout = 1000;
        while (timeout != 0) {
        #ifdef __WIN32
            SleepEx(1,true);
        #else
	    usleep(1000);
        #endif
	    highlight_definition = hl_manager->definition( hl_manager->definitionIdByName(LANG_NAME) );
            if (!highlight_definition.isNull())
                break;
            timeout --;
        }
#endif
        if (!highlight_definition.isNull())
		highlighter->setDefaultContext(highlight_definition->initialContext());

	if (argc == 1) {
		load_text(TEST_FILE, text_editor);
	} else {
		load_text(argv[1], text_editor);
	}
	main_window->setWindowTitle("Kate syntax highter test");
	main_window->setCentralWidget(text_editor);
	main_window->show();
	return app.exec();
}

void load_text(QString fe, QPlainTextEdit *te )
{
	QFile f(fe);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QString s = f.readAll();
	te->setPlainText(s);
	te->setLineWrapMode(QPlainTextEdit::NoWrap);
}
